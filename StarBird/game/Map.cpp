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

Map::Map(Scene& scene, const glm::ivec2& mapDimensions, const bool singleEntryPoint)
    : mScene(scene)
    , mMapDimensions(mapDimensions)
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
        SceneObject starSO;

        starSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + (mapNodeEntry.second.mNodeType == Map::NodeType::BOSS_ENCOUNTER ? "octo_star.bmp" : "quad_star.bmp")), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.5f), false);
        starSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        starSO.mName = strutils::StringId("star_" + std::to_string(positionCounter));
        starSO.mScale.x = 1.5f;
        starSO.mScale.y = 1.5f;
        starSO.mPosition = mapNodeEntry.second.mPosition;
        starSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(starSO));
        positionCounter++;
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
                
                pathSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "star_path.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), pathSO.mScale, false);
                pathSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                pathSO.mPosition = mMapData.at(mapNodeEntry.first).mPosition + dirToNext * (i/static_cast<float>(pathSegments));
                pathSO.mScale = glm::vec3(0.25f, 0.25f, 1.0f);
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
