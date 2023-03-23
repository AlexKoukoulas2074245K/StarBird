///------------------------------------------------------------------------------------------------
///  Map.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 22/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Map.h"
#include "GameObjectConstants.h"
#include "SceneObject.h"
#include "Scene.h"
#include "../resloading/ResourceLoadingService.h"

///------------------------------------------------------------------------------------------------

Map::Map(Scene& scene, const glm::ivec2& mapDimensions, const MapCoord& currentMapCoord, const bool singleEntryPoint)
    : mScene(scene)
    , mMapDimensions(mapDimensions)
    , mCurrentMapCoord(currentMapCoord)
    , mHasSingleEntryPoint(singleEntryPoint)
{
    GenerateMapData();
    CreateMapSceneObjects();
}

///------------------------------------------------------------------------------------------------

const std::map<MapCoord, Map::NodeData>& Map::GetMapData() const
{
    return mMapData;
}

///------------------------------------------------------------------------------------------------

void Map::GenerateMapData()
{
    const int iterations = 4;
    
    for (int i = 0; i < iterations; ++i)
    {
        auto currentCoordinate = mHasSingleEntryPoint ? MapCoord(0, mMapDimensions.y/2) : MapCoord(0, math::RandomInt(0, mMapDimensions.y - 1));
        mMapData[currentCoordinate].mPosition = GenerateNodePositionFromCoord(currentCoordinate);
        
        for (int col = 1; col < mMapDimensions.x; ++col)
        {
            MapCoord targetCoord = RandomlySelectNextMapCoord(currentCoordinate);
            
            while (DetectedCrossedEdge(currentCoordinate, targetCoord))
            {
                targetCoord = RandomlySelectNextMapCoord(currentCoordinate);
            }
            
            mMapData[currentCoordinate].mNodeLinks.insert(targetCoord);
            currentCoordinate = targetCoord;
            mMapData[currentCoordinate].mPosition = GenerateNodePositionFromCoord(currentCoordinate);
            mMapData[currentCoordinate].mNodeType = math::RandomSign() == -1 ? NodeType::NORMAL_ENCOUNTER : NodeType::BOSS_ENCOUNTER;
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
        bgSO.mScale = game_object_constants::BACKGROUND_SCALE;
        bgSO.mScale.x *= 2.0f;
        bgSO.mScale.y *= 2.0f;
        bgSO.mPosition.z = game_object_constants::BACKGROUND_Z;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = scene_object_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    auto positionCounter = 0;
    for (const auto& mapNodeEntry: mMapData)
    {
        SceneObject planetSO;
        
        if (mapNodeEntry.first == MapCoord(8, 2))
        {
            for (int i = 0; i < 2; ++i)
            {
                planetSO.mAnimation = std::make_unique<NebulaAnimation>(nullptr, resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "noise_" + std::to_string(i) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::NEBULA_SHADER_FILE_NAME), glm::vec3(3.0f), scene_object_constants::NEBULA_ANIMATION_SPEED, false);
                
                planetSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                planetSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                planetSO.mScale = glm::vec3(3.0f);
                planetSO.mPosition = mapNodeEntry.second.mPosition;
                mScene.AddSceneObject(std::move(planetSO));
            }
        }
        else
        {
            if (mMapData.at(mCurrentMapCoord).mNodeLinks.contains(mapNodeEntry.first))
            {
                planetSO.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::MAP_PLANET_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::MAP_PLANET_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::HUE_SHIFT_SHADER_FILE_NAME), glm::vec3(0.75f), 0.0f, 0.005f, 1.0f/200.0f, false);
            }
            else
            {
                planetSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::MAP_PLANET_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::MAP_PLANET_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::HUE_SHIFT_SHADER_FILE_NAME), glm::vec3(0.75f), false);
                
                {
                    SceneObject planetRingSO;
                    
                    planetRingSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::MAP_PLANET_RING_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::MAP_PLANET_RING_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
                    planetRingSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                    planetRingSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                    planetRingSO.mScale = glm::vec3(1.0f);
                    planetRingSO.mRotation.x = -math::PI/4;
                    planetRingSO.mPosition = mapNodeEntry.second.mPosition;
                    mScene.AddSceneObject(std::move(planetRingSO));
                }
            }
            
            planetSO.mShaderFloatUniformValues[scene_object_constants::HUE_SHIFT_UNIFORM_NAME] = math::RandomFloat(0, 2.0f * math::PI);
            planetSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
            planetSO.mSceneObjectType = SceneObjectType::WorldGameObject;
            planetSO.mScale = glm::vec3(0.75f);
            planetSO.mPosition = mapNodeEntry.second.mPosition;
            planetSO.mName = strutils::StringId("PLANET_" + std::to_string(positionCounter++));
            mScene.AddSceneObject(std::move(planetSO));
        }
        
        
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
                    pathSO.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "star_path.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(0.3f, 0.3f, 1.0f), 100.0f * i, 0.01f, 1.0f/100.0f, false);
                }
                else
                {
                    pathSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "star_path.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), pathSO.mScale, false);
                }
                
                pathSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                pathSO.mPosition = mMapData.at(mapNodeEntry.first).mPosition + dirToNext * (i/static_cast<float>(pathSegments));
                pathSO.mScale = glm::vec3(0.3f, 0.3f, 1.0f);
                pathSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
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

glm::vec3 Map::GenerateNodePositionFromCoord(const MapCoord& mapCoord) const
{
    // Base calculation
    glm::vec3 result = glm::vec3
    (
        game_object_constants::MAP_MIN_WORLD_BOUNDS.x + 4.0f * mapCoord.mCol, // Base horizontal spacing
        game_object_constants::MAP_MIN_WORLD_BOUNDS.y + 10.0f - mapCoord.mRow * 5.0f + mapCoord.mCol * 2.0f, // Base vertical alignment + staircase increment
        0.0f
     );
    
    // Add noise
    result.x += math::RandomFloat(-1.0f, 1.0f);
    result.y += math::RandomFloat(-1.0f, 1.0f);
    
    return result;
}

///------------------------------------------------------------------------------------------------

MapCoord Map::RandomlySelectNextMapCoord(const MapCoord& currentMapCoord) const
{
    auto randRow = math::Max(math::Min(mMapDimensions.y - 1, currentMapCoord.mRow + math::RandomInt(-1, 1)), 0);
    return currentMapCoord.mCol == mMapDimensions.x - 2 ? MapCoord(mMapDimensions.x - 1, mMapDimensions.y/2) : MapCoord(currentMapCoord.mCol + 1, randRow);
}

///------------------------------------------------------------------------------------------------
