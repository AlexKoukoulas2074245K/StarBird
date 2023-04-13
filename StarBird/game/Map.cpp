///------------------------------------------------------------------------------------------------
///  Map.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 22/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Map.h"
#include "GameConstants.h"
#include "SceneObject.h"
#include "Scene.h"
#include "../resloading/ResourceLoadingService.h"

#include <unordered_set>

///------------------------------------------------------------------------------------------------

static const char* MAP_PLANET_MESH_FILE_NAME = "planet.obj";
static const char* MAP_PLANET_RING_MESH_FILE_NAME = "planet_ring.obj";
static const char* MAP_LAB_MESH_FILE_NAME = "base.obj";

static const glm::vec3 MAP_NEBULA_NODE_SCALE = glm::vec3(3.0f, 3.0f, 1.0f);
static const glm::vec3 MAP_STAR_PATH_SCALE = glm::vec3(0.3f, 0.3f, 1.0f);
static const glm::vec3 MAP_LAB_SCALE = glm::vec3(0.9, 0.5f, 0.9f);

static const float MAP_BASE_X_ROTATION = 0.6f;
static const float MAP_STAR_PATH_PULSING_DELAY_MILLIS = 100.0f;
static const float MAP_STAR_PATH_PULSING_SPEED = 0.01f;
static const float MAP_STAR_PATH_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;
static const float MAP_PLANET_RING_MIN_X_ROTATION = 1.8f;
static const float MAP_PLANET_RING_MAX_X_ROTATION = 2.2f;
static const float MAP_PLANET_RING_MIN_Y_ROTATION = -math::PI/10;
static const float MAP_PLANET_RING_MAX_Y_ROTATION = +math::PI/10;
static const float MAP_NODE_ROTATION_SPEED = 0.0002f;
static const float MAP_NODE_PULSING_SPEED = 0.005f;
static const float MAP_NODE_PULSING_ENLARGEMENT_FACTOR = 1.0f/200.0f;

///------------------------------------------------------------------------------------------------

Map::Map(Scene& scene, const int generationSeed, const glm::ivec2& mapDimensions, const MapCoord& currentMapCoord, const bool singleEntryPoint)
    : mScene(scene)
    , mMapDimensions(mapDimensions)
    , mCurrentMapCoord(currentMapCoord)
    , mGenerationSeed(generationSeed)
    , mHasSingleEntryPoint(singleEntryPoint)
{
    math::SetControlSeed(generationSeed);
    
    // No map data has been saved to GameSingletons
    if (mMapData.empty())
    {
        GenerateMapData();
    }
    
    CreateMapSceneObjects();
}

///------------------------------------------------------------------------------------------------

int Map::GetCurrentGenerationSeed() const
{
    return mGenerationSeed;
}

///------------------------------------------------------------------------------------------------

const std::map<MapCoord, Map::NodeData>& Map::GetMapData() const
{
    return mMapData;
}

///------------------------------------------------------------------------------------------------

const glm::ivec2& Map::GetMapDimensions() const
{
    return mMapDimensions;
}

///------------------------------------------------------------------------------------------------

void Map::GenerateMapData()
{
    const int iterations = 4;
    
    for (int i = 0; i < iterations; ++i)
    {
        auto currentCoordinate = mHasSingleEntryPoint ? MapCoord(0, mMapDimensions.y/2) : MapCoord(0, math::ControlledRandomInt(0, mMapDimensions.y - 1));
        mMapData[currentCoordinate].mPosition = GenerateNodePositionForCoord(currentCoordinate);
        mMapData[currentCoordinate].mNodeType = SelectNodeTypeForCoord(currentCoordinate);
        
        for (int col = 1; col < mMapDimensions.x; ++col)
        {
            MapCoord targetCoord = RandomlySelectNextMapCoord(currentCoordinate);
            
            while (DetectedCrossedEdge(currentCoordinate, targetCoord))
            {
                targetCoord = RandomlySelectNextMapCoord(currentCoordinate);
            }
            
            mMapData[currentCoordinate].mNodeLinks.insert(targetCoord);
            currentCoordinate = targetCoord;
            mMapData[currentCoordinate].mPosition = GenerateNodePositionForCoord(currentCoordinate);
            mMapData[currentCoordinate].mNodeType = SelectNodeTypeForCoord(currentCoordinate);
        }
    }
}

///------------------------------------------------------------------------------------------------

void Map::CreateMapSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_constants::MAP_BACKGROUND_SCALE;
        bgSO.mPosition.z = game_constants::BACKGROUND_Z;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // All node meshes
    for (const auto& mapNodeEntry: mMapData)
    {
        SceneObject nodeSo;
        nodeSo.mName = strutils::StringId(mapNodeEntry.first.ToString());
        nodeSo.mPosition = mapNodeEntry.second.mPosition;
        
        switch (mapNodeEntry.second.mNodeType)
        {
            case NodeType::HARD_ENCOUNTER:
            {
                SceneObject planetRingSO;
                
                auto shaderNameToUse = mapNodeEntry.first.mCol <= mCurrentMapCoord.mCol ? game_constants::GRAYSCALE_SHADER_FILE_NAME : game_constants::BASIC_SHADER_FILE_NAME;
                planetRingSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_PLANET_RING_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + MAP_PLANET_RING_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + shaderNameToUse), glm::vec3(1.0f), false);
                planetRingSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                planetRingSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                planetRingSO.mScale = glm::vec3(1.0f);
                planetRingSO.mRotation.x = math::ControlledRandomFloat(MAP_PLANET_RING_MIN_X_ROTATION, MAP_PLANET_RING_MAX_X_ROTATION);
                planetRingSO.mRotation.y += math::ControlledRandomFloat(MAP_PLANET_RING_MIN_Y_ROTATION, MAP_PLANET_RING_MAX_Y_ROTATION);
                planetRingSO.mPosition = mapNodeEntry.second.mPosition;
                planetRingSO.mName = strutils::StringId("PLANET_RING_" + mapNodeEntry.first.ToString());
                
                // Add also pulsing animation if node is active
                if (mMapData.at(mCurrentMapCoord).mNodeLinks.contains(mapNodeEntry.first))
                {
                    planetRingSO.mExtraCompoundingAnimations.push_back( std::make_unique<PulsingAnimation>(planetRingSO.mAnimation->VGetCurrentTextureResourceId(), planetRingSO.mAnimation->VGetCurrentMeshResourceId(), planetRingSO.mAnimation->VGetCurrentShaderResourceId(), planetRingSO.mAnimation->VGetScale(),  PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, MAP_NODE_PULSING_SPEED, MAP_NODE_PULSING_ENLARGEMENT_FACTOR, false));
                }
                
                mScene.AddSceneObject(std::move(planetRingSO));
            } // Intentional Fallthrough
            case NodeType::NORMAL_ENCOUNTER:
            {
                bool shouldRotate = mapNodeEntry.first.mCol > mCurrentMapCoord.mCol;
                
                nodeSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_PLANET_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + MAP_PLANET_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + (shouldRotate ? game_constants::HUE_SHIFT_SHADER_FILE_NAME : game_constants::GRAYSCALE_SHADER_FILE_NAME)), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f,  shouldRotate ? MAP_NODE_ROTATION_SPEED : 0.0f, false);
                
                nodeSo.mShaderFloatUniformValues[game_constants::HUE_SHIFT_UNIFORM_NAME] = math::ControlledRandomFloat(0, 2.0f * math::PI);
                nodeSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
            } break;
               
            case NodeType::LAB:
            {
                bool shouldRotate = mapNodeEntry.first.mCol > mCurrentMapCoord.mCol;
                
                nodeSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_BASE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + MAP_LAB_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + (shouldRotate ? game_constants::BASIC_SHADER_FILE_NAME : game_constants::GRAYSCALE_SHADER_FILE_NAME)), MAP_LAB_SCALE, RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, shouldRotate ? MAP_NODE_ROTATION_SPEED : 0.0f, false);
                nodeSo.mRotation.x = MAP_BASE_X_ROTATION;
                nodeSo.mScale = MAP_LAB_SCALE;
            } break;
                
            case NodeType::BOSS_ENCOUNTER:
            {
                // Nebula
                for (int i = 0; i < 2; ++i)
                {
                    nodeSo.mAnimation = std::make_unique<NebulaAnimation>(nullptr, resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::NOISE_PREFIX_TEXTURE_FILE_NAME + std::to_string(i) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BLACK_NEBULA_SHADER_FILE_NAME), MAP_NEBULA_NODE_SCALE, game_constants::NEBULA_ANIMATION_SPEED, false);
                    
                    nodeSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                    nodeSo.mSceneObjectType = SceneObjectType::WorldGameObject;
                    nodeSo.mScale = MAP_NEBULA_NODE_SCALE;
                }
            } break;
                
            default: break;
        }
        
        // Add also pulsing animation if node is active
        if (mMapData.at(mCurrentMapCoord).mNodeLinks.contains(mapNodeEntry.first))
        {
            nodeSo.mExtraCompoundingAnimations.push_back( std::make_unique<PulsingAnimation>(nodeSo.mAnimation->VGetCurrentTextureResourceId(), nodeSo.mAnimation->VGetCurrentMeshResourceId(), nodeSo.mAnimation->VGetCurrentShaderResourceId(), nodeSo.mAnimation->VGetScale(),  PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, MAP_NODE_PULSING_SPEED, MAP_NODE_PULSING_ENLARGEMENT_FACTOR, false));
        }
        
        mScene.AddSceneObject(std::move(nodeSo));
    }

    for (const auto& mapNodeEntry: mMapData)
    {
        for (const auto& linkedCoord: mapNodeEntry.second.mNodeLinks)
        {
            glm::vec3 dirToNext = mMapData.at(linkedCoord).mPosition - mMapData.at(mapNodeEntry.first).mPosition;
            
            auto pathSegments = 2 * static_cast<int>(glm::length(dirToNext));
            for (int i = 0; i < pathSegments; ++i)
            {
                SceneObject pathSO;
                
                if (mapNodeEntry.first == mCurrentMapCoord)
                {
                    pathSO.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_STAR_PATH_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), MAP_STAR_PATH_SCALE,  PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, MAP_STAR_PATH_PULSING_DELAY_MILLIS * i, MAP_STAR_PATH_PULSING_SPEED, MAP_STAR_PATH_PULSING_ENLARGEMENT_FACTOR, false);
                }
                else
                {
                    pathSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_STAR_PATH_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME), pathSO.mScale, false);
                    
                    pathSO.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = glm::vec4(0.5f, 0.5f, 0.5f, 0.8f);
                }
                
                
                pathSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                pathSO.mPosition = mMapData.at(mapNodeEntry.first).mPosition + dirToNext * (i/static_cast<float>(pathSegments));
                pathSO.mScale = MAP_STAR_PATH_SCALE;
                pathSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                mScene.AddSceneObject(std::move(pathSO));
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

bool Map::DetectedCrossedEdge(const MapCoord& currentCoord, const MapCoord& targetTestCoord) const
{
    bool currentCoordHasTopNeighbor = currentCoord.mRow > 0;
    bool currentCoordHasBotNeighbor = currentCoord.mRow < mMapDimensions.y - 1;
    bool targetCoordHasTopNeighbor = targetTestCoord.mRow > 0;
    bool targetCoordHasBotNeighbor = targetTestCoord.mRow < mMapDimensions.y - 1;
    
    if (currentCoordHasTopNeighbor && targetCoordHasBotNeighbor)
    {
        MapCoord currentTopNeighbor(currentCoord.mCol, currentCoord.mRow - 1);
        if (mMapData.count(currentTopNeighbor) && mMapData.at(currentTopNeighbor).mNodeLinks.contains(MapCoord(targetTestCoord.mCol, targetTestCoord.mRow + 1))) return true;
    }
    if (currentCoordHasBotNeighbor && targetCoordHasTopNeighbor)
    {
        MapCoord currentBotNeighbor(currentCoord.mCol, currentCoord.mRow + 1);
        if (mMapData.count(currentBotNeighbor) && mMapData.at(currentBotNeighbor).mNodeLinks.contains(MapCoord(targetTestCoord.mCol, targetTestCoord.mRow - 1))) return true;
    }
    
    return false;
}

///------------------------------------------------------------------------------------------------

glm::vec3 Map::GenerateNodePositionForCoord(const MapCoord& currentMapCoord) const
{
    // Base calculation
    glm::vec3 result = glm::vec3
    (
        game_constants::MAP_MIN_WORLD_BOUNDS.x + 7.0f * currentMapCoord.mCol, // Base horizontal spacing
        game_constants::MAP_MIN_WORLD_BOUNDS.y + 10.0f - currentMapCoord.mRow * 5.0f + currentMapCoord.mCol * 4.0f, // Base vertical alignment + staircase increment
        0.0f
     );
    
    // Add noise
    result.x += math::ControlledRandomFloat(-1.0f, 1.0f);
    result.y += math::ControlledRandomFloat(-1.0f, 1.0f);
    return result;
}

///------------------------------------------------------------------------------------------------

Map::NodeType Map::SelectNodeTypeForCoord(const MapCoord& currentMapCoord) const
{
    // Forced single entry point and starting coord case
    if (mHasSingleEntryPoint && currentMapCoord == MapCoord(0, mMapDimensions.y/2))
    {
        return NodeType::NORMAL_ENCOUNTER;
    }
    // Last map coord
    else if (currentMapCoord == MapCoord(mMapDimensions.x - 1, mMapDimensions.y/2))
    {
        return NodeType::BOSS_ENCOUNTER;
    }
    else
    {
        // Generate list of node types to pick from
        std::unordered_set<NodeType> availableNodeTypes;
        for (int i = 0; i < static_cast<int>(NodeType::COUNT); ++i)
        {
            availableNodeTypes.insert(static_cast<NodeType>(i));
        }
        
        // Only last node can have a boss encounter
        availableNodeTypes.erase(NodeType::BOSS_ENCOUNTER);
        
        // Second node can not have base
        if (currentMapCoord.mCol == 1)
        {
            //availableNodeTypes.erase(NodeType::LAB);
            availableNodeTypes.clear();
            availableNodeTypes.insert(NodeType::LAB);
        }
        
        // Remove any node types from the immediate previous links except if there are
        // normal encounters
        for (const auto& mapEntry: mMapData)
        {
            if (mapEntry.second.mNodeType == NodeType::NORMAL_ENCOUNTER) continue;
            
            if (mapEntry.second.mNodeLinks.contains(currentMapCoord))
            {
                availableNodeTypes.erase(mapEntry.second.mNodeType);
            }
        }
        
        // Select at random from the remaining node types.
        // Unfortunately because it's a set I can't just pick begin() + random index
        auto randomIndex = math::ControlledRandomInt(0, static_cast<int>(availableNodeTypes.size()) - 1);
        for (const auto& nodeType: availableNodeTypes)
        {
            if (randomIndex-- == 0) return nodeType;
        }
    }
    
    return NodeType::NORMAL_ENCOUNTER;
}

///------------------------------------------------------------------------------------------------

MapCoord Map::RandomlySelectNextMapCoord(const MapCoord& currentMapCoord) const
{
    auto randRow = math::Max(math::Min(mMapDimensions.y - 1, currentMapCoord.mRow + math::ControlledRandomInt(-1, 1)), 0);
    return currentMapCoord.mCol == mMapDimensions.x - 2 ? MapCoord(mMapDimensions.x - 1, mMapDimensions.y/2) : MapCoord(currentMapCoord.mCol + 1, randRow);
}

///------------------------------------------------------------------------------------------------
