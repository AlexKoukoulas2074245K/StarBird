///------------------------------------------------------------------------------------------------
///  Map.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 22/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Map.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "ObjectiveCUtils.h"
#include "SceneObject.h"
#include "Scene.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"
#include "datarepos/WaveBlocksRepository.h"
#include <SDL.h>
#include <fstream>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

static const char* STARTING_LOCATION_TEXTURE_FILE_NAME = "octo_star.bmp";
static const char* LAB_TEXTURE_FILE_NAME = "lab_mm.bmp";
static const char* EVENT_TEXTURE_FILE_NAME = "event_mm.bmp";
static const char* MAP_PATH_NAME_SUFFIX = "_PATH";

static const glm::vec3 MAP_NEBULA_NODE_SCALE = glm::vec3(3.0f, 3.0f, 1.0f);
static const glm::vec3 MAP_LAB_SCALE = glm::vec3(0.9, 0.5f, 0.9f);
static const glm::vec3 LAB_SCALE = glm::vec3(2.5f, 2.5f, 1.0f);
static const glm::vec3 EVENT_SCALE = glm::vec3(2.5f, 2.5f, 1.0f);
static const glm::vec3 STARTING_LOCATION_SCALE = glm::vec3(4.0, 4.0f, 1.0f);

static const float MAP_BASE_X_ROTATION = 0.6f;
static const float MAP_PLANET_RING_MIN_X_ROTATION = 1.8f;
static const float MAP_PLANET_RING_MAX_X_ROTATION = 2.2f;
static const float MAP_PLANET_RING_MIN_Y_ROTATION = -math::PI/10;
static const float MAP_PLANET_RING_MAX_Y_ROTATION = +math::PI/10;

static const float LEVEL_WAVE_Y_INCREMENT = 2.0f;

///------------------------------------------------------------------------------------------------

Map::Map(Scene& scene, const int generationSeed, const glm::ivec2& mapDimensions, const MapCoord& currentMapCoord, const bool singleEntryPoint)
    : mScene(scene)
    , mMapDimensions(mapDimensions)
    , mCurrentMapCoord(currentMapCoord)
    , mGenerationSeed(generationSeed)
    , mHasSingleEntryPoint(singleEntryPoint)
{
    math::SetControlSeed(generationSeed);
    GenerateMapData();
    CreateMapSceneObjects();
    CreateLevelFiles();
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
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BACKGROUND_TEXTURE_FILE_PATH + std::to_string(GameSingletons::GetBackgroundIndex()) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
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
            case NodeType::STARTING_LOCATION:
            {
                nodeSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + STARTING_LOCATION_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), STARTING_LOCATION_SCALE, false);
                nodeSo.mScale = STARTING_LOCATION_SCALE;
            } break;
                
            case NodeType::HARD_ENCOUNTER:
            {
                SceneObject planetRingSO;
                
                auto shaderNameToUse = mapNodeEntry.first.mCol <= mCurrentMapCoord.mCol ? game_constants::GRAYSCALE_SHADER_FILE_NAME : game_constants::BASIC_SHADER_FILE_NAME;
                planetRingSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_PLANET_RING_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::MAP_PLANET_RING_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + shaderNameToUse), glm::vec3(1.0f), false);
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
                    planetRingSO.mExtraCompoundingAnimations.push_back( std::make_unique<PulsingAnimation>(planetRingSO.mAnimation->VGetCurrentTextureResourceId(), planetRingSO.mAnimation->VGetCurrentMeshResourceId(), planetRingSO.mAnimation->VGetCurrentShaderResourceId(), planetRingSO.mAnimation->VGetScale(),  PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, game_constants::MAP_NODE_PULSING_SPEED, game_constants::MAP_NODE_PULSING_ENLARGEMENT_FACTOR, false));
                }
                
                mScene.AddSceneObject(std::move(planetRingSO));
            } // Intentional Fallthrough
            case NodeType::NORMAL_ENCOUNTER:
            {
                bool shouldRotate = mapNodeEntry.first.mCol > mCurrentMapCoord.mCol;
                
                nodeSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_PLANET_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::MAP_PLANET_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + (shouldRotate ? game_constants::HUE_SHIFT_SHADER_FILE_NAME : game_constants::GRAYSCALE_SHADER_FILE_NAME)), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f,  shouldRotate ? game_constants::MAP_NODE_ROTATION_SPEED : 0.0f, false);
                
                nodeSo.mShaderFloatUniformValues[game_constants::HUE_SHIFT_UNIFORM_NAME] = math::ControlledRandomFloat(0, 2.0f * math::PI);
                nodeSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
            } break;
            
            case NodeType::EVENT:
            {
                bool shouldUseGrayscale = mapNodeEntry.first.mCol <= mCurrentMapCoord.mCol;
                nodeSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + EVENT_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + (shouldUseGrayscale ? game_constants::GRAYSCALE_SHADER_FILE_NAME : game_constants::BASIC_SHADER_FILE_NAME)), EVENT_SCALE, false);
                nodeSo.mScale = EVENT_SCALE;
            } break;
                
            case NodeType::LAB:
            {
                bool shouldUseGrayscale = mapNodeEntry.first.mCol <= mCurrentMapCoord.mCol;
                nodeSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + LAB_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + (shouldUseGrayscale ? game_constants::GRAYSCALE_SHADER_FILE_NAME : game_constants::BASIC_SHADER_FILE_NAME)), LAB_SCALE, false);
                nodeSo.mScale = LAB_SCALE;
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
            nodeSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(nodeSo.mAnimation->VGetCurrentTextureResourceId(), nodeSo.mAnimation->VGetCurrentMeshResourceId(), nodeSo.mAnimation->VGetCurrentShaderResourceId(), nodeSo.mAnimation->VGetScale(),  PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, game_constants::MAP_NODE_PULSING_SPEED, game_constants::MAP_NODE_PULSING_ENLARGEMENT_FACTOR, false));
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
                    pathSO.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_STAR_PATH_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), game_constants::MAP_STAR_PATH_SCALE,  PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, game_constants::MAP_STAR_PATH_PULSING_DELAY_MILLIS * i, game_constants::MAP_STAR_PATH_PULSING_SPEED, game_constants::MAP_STAR_PATH_PULSING_ENLARGEMENT_FACTOR, false);
                }
                else
                {
                    pathSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MAP_STAR_PATH_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::GRAYSCALE_SHADER_FILE_NAME), pathSO.mScale, false);
                }
                
                
                pathSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                pathSO.mPosition = mMapData.at(mapNodeEntry.first).mPosition + dirToNext * (i/static_cast<float>(pathSegments));
                pathSO.mScale = game_constants::MAP_STAR_PATH_SCALE;
                pathSO.mName = strutils::StringId(mapNodeEntry.first.ToString() + "-" + linkedCoord.ToString() + "_" + std::to_string(i) + MAP_PATH_NAME_SUFFIX);
                pathSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                mScene.AddSceneObject(std::move(pathSO));
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void Map::CreateLevelFiles()
{
    auto startTime = SDL_GetPerformanceCounter();
    
    for (const auto& mapNodeEntry: mMapData)
    {
        switch (mapNodeEntry.second.mNodeType)
        {
            case NodeType::NORMAL_ENCOUNTER:
            case NodeType::HARD_ENCOUNTER:
            case NodeType::BOSS_ENCOUNTER: GenerateLevelWaves(mapNodeEntry.first, mapNodeEntry.second);
            
            default: break;
        }
    }
    
    auto currTime = SDL_GetPerformanceCounter();
    
    double elapsedTime = static_cast<double>(
      (currTime - startTime) / static_cast<double>(SDL_GetPerformanceFrequency())
    );
    
    Log(LogType::INFO, "Level generation finished in %.6f millis", elapsedTime * 1000.0f);
}

///------------------------------------------------------------------------------------------------

void Map::GenerateLevelWaves(const MapCoord& mapCoord, const NodeData& nodeData)
{
    Log(LogType::INFO, "Generating level for map node %s", mapCoord.ToString().c_str());
    
    std::stringstream levelXml;
    levelXml <<
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "\n<Level>"
        "\n<Camera type=\"world_cam\" lenseHeight=\"30.0f\"/>"
        "\n<Camera type=\"gui_cam\" lenseHeight=\"30.0f\"/>";
    
    
    auto difficultyValue = mapCoord.mCol;
    if (nodeData.mNodeType == NodeType::BOSS_ENCOUNTER)
    {
        difficultyValue *= 1.5f;
    }
    else if (nodeData.mNodeType == NodeType::HARD_ENCOUNTER)
    {
        difficultyValue *= 2.0f;
    }
    
    difficultyValue += GameSingletons::GetMapLevel() * 10;
    
    auto waveCount = math::ControlledRandomInt(2,3) + (difficultyValue / 5);
    
    const auto& eligibleBlocks = WaveBlocksRepository::GetInstance().GetEligibleWaveBlocksForDifficulty(difficultyValue);
    
    if (eligibleBlocks.size() == 0) return;
    
    for (int j = 0; j < waveCount; ++j)
    {
        levelXml << "\n    <Wave";
        auto selectedBlockIndex = math::ControlledRandomInt(0, static_cast<int>(eligibleBlocks.size()) - 1);
        auto selectedBlock = eligibleBlocks.at(selectedBlockIndex);
        if (selectedBlock.mExtensible)
        {
            ExtendWaveBlockForDifficulty(difficultyValue, selectedBlock);
        }
        
        levelXml << " blockIndex=\"" << std::to_string(selectedBlockIndex) << "\" difficulty=\"" << std::to_string(difficultyValue) << "\"";
        
        if (nodeData.mNodeType == NodeType::BOSS_ENCOUNTER && j == waveCount - 1)
        {
            selectedBlock = WaveBlocksRepository::GetInstance().GetBossWaveBlock(strutils::StringId("Ka'thun"));
            levelXml << " bossName=\"" << selectedBlock.mBossName.GetString() << "\" bossHealth=\"" << selectedBlock.mBossHealth << "\"";
        }
        
        levelXml << ">";
        for (const auto& blockLine: selectedBlock.mWaveBlockLines)
        {
            for (const auto& enemy: blockLine.mEnemies)
            {
                auto positionOffset = selectedBlock.mInflexible ? glm::vec2(0.0f, 0.0f) : glm::vec2(math::RandomFloat(-1.0f, 1.0f), math::RandomFloat(-1.0f, 1.0f));
                levelXml << "\n        <Enemy position=\"" << enemy.mPosition.x + positionOffset.x << ", " << enemy.mPosition.y + positionOffset.y << "\" type=\"" << enemy.mGameObjectEnemyType.GetString() << "\"/>";
            }
        }
        
        levelXml << "\n    </Wave>";
    }
    
    levelXml << "\n</Level>";
    
    auto levelFileName = objectiveC_utils::BuildLocalFileSaveLocation(mapCoord.ToString() + ".xml");
    std::ofstream outputFile(levelFileName);
    outputFile << levelXml.str();
    outputFile.close();
}

///------------------------------------------------------------------------------------------------

void Map::ExtendWaveBlockForDifficulty(const int difficulty, WaveBlockDefinition& waveBlock) const
{
    if (difficulty == waveBlock.mDifficulty) return;
    
    // Find largest y in block line enemies
    auto waveHeight = 0.0f;
    auto riter = waveBlock.mWaveBlockLines.rbegin();
    while (riter != waveBlock.mWaveBlockLines.rend())
    {
        if (!riter->mEnemies.empty())
        {
            waveHeight = riter->mEnemies.back().mPosition.y - game_constants::LEVEL_WAVE_VISIBLE_Y;
            break;
        }
        
        riter++;
    }
    
    auto currentY = game_constants::LEVEL_WAVE_VISIBLE_Y + waveHeight + LEVEL_WAVE_Y_INCREMENT;
    
    std::vector<WaveBlockLine> additionalLines;
    auto difficultyDiff = difficulty - waveBlock.mDifficulty;
    for (int i = 0; i < difficultyDiff; ++i)
    {
        auto targetLineCopy = waveBlock.mWaveBlockLines.at(i % waveBlock.mWaveBlockLines.size());
        auto lineHeight = GetWaveBlockLineHeight(targetLineCopy);
        
        for (auto& enemy: targetLineCopy.mEnemies)
        {
            enemy.mPosition.y = currentY + enemy.mPosition.y - game_constants::LEVEL_WAVE_VISIBLE_Y;
        }
        
        currentY += lineHeight;
        
        additionalLines.push_back(targetLineCopy);
    }
    
    waveBlock.mWaveBlockLines.insert(waveBlock.mWaveBlockLines.end(), additionalLines.begin(), additionalLines.end());
}

///------------------------------------------------------------------------------------------------

float Map::GetWaveBlockLineHeight(const WaveBlockLine& waveBlockLine) const
{
    return waveBlockLine.mEnemies.empty() ? 0.0f : waveBlockLine.mEnemies.back().mPosition.y - game_constants::LEVEL_WAVE_VISIBLE_Y + LEVEL_WAVE_Y_INCREMENT;
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
        return NodeType::STARTING_LOCATION;
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
        
        // Only first node is a starting location
        availableNodeTypes.erase(NodeType::STARTING_LOCATION);
        
        // Only last node can have a boss encounter
        availableNodeTypes.erase(NodeType::BOSS_ENCOUNTER);
        
        // Second node can not have base
        if (currentMapCoord.mCol == 1)
        {
            availableNodeTypes.erase(NodeType::LAB);
        }
        
        // Remove any node types from the immediate previous links except if they are
        // normal encounters or events
        for (const auto& mapEntry: mMapData)
        {
            if (mapEntry.second.mNodeType == NodeType::NORMAL_ENCOUNTER || mapEntry.second.mNodeType == NodeType::EVENT) continue;
            
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
