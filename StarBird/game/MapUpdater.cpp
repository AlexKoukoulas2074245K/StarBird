///------------------------------------------------------------------------------------------------
///  MapUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "MapUpdater.h"
#include "GameObjectConstants.h"
#include "GameSingletons.h"
#include "SceneObjectConstants.h"
#include "Scene.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

#include <vector>
#include <map>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

static const float MAX_MAP_VELOCITY_LENGTH = 2.0f;
static const float MAP_VELOCITY_DAMPING = 0.9f;
static const float MAP_VELOCITY_INTEGRATION_SPEED = 0.04f;
static const float CAMERA_MAX_ZOOM_FACTOR = 2.4f;
static const float CAMERA_ZOOM_SPEED = 0.1f;
static const float MIN_CAMERA_VELOCITY_TO_START_MOVEMENT = 0.01f;

static const glm::vec2 MAP_MAX_BOUNDS = glm::vec2(17.0f, 13.0f);
static const glm::vec2 MAP_MIN_BOUNDS = glm::vec2(-17.0f, -13.0f);

///------------------------------------------------------------------------------------------------

struct MapCoord
{
    MapCoord(const int col, const int row)
        : mCol(col)
        , mRow(row)
    {
    }
    
    int mCol;
    int mRow;
};

bool operator < (const MapCoord& lhs, const MapCoord& rhs)
{
    if (lhs.mCol == rhs.mCol)
    {
        return lhs.mRow < rhs.mRow;
    }
    else
    {
        return lhs.mCol < rhs.mCol;
    }
}

bool operator == (const MapCoord& lhs, const MapCoord& rhs)
{
    return lhs.mCol == rhs.mCol && lhs.mRow == rhs.mRow;
}

struct MapCoordHasher
{
    std::size_t operator()(const MapCoord& key) const
    {
        return strutils::StringId(std::to_string(key.mCol) + "," + std::to_string(key.mRow)).GetStringId();
    }
};

bool DetectedCrossedEdge(const MapCoord& currentCoord, const MapCoord& targetTestCoord, const std::map<MapCoord, std::unordered_set<MapCoord, MapCoordHasher>>& generatedLinks, const int mapRows)
{
    bool currentCoordHasTopNeighbor = currentCoord.mRow > 0;
    bool currentCoordHasBotNeighbor = currentCoord.mRow < mapRows - 1;
    bool targetCoordHasTopNeighbor = targetTestCoord.mRow > 0;
    bool targetCoordHasBotNeighbor = targetTestCoord.mRow < mapRows - 1;
    
    if (currentCoordHasTopNeighbor && targetCoordHasBotNeighbor)
    {
        auto currentCoordTopNeighborLinksIter = generatedLinks.find(MapCoord(currentCoord.mCol, currentCoord.mRow - 1));
        if (currentCoordTopNeighborLinksIter != generatedLinks.cend() && currentCoordTopNeighborLinksIter->second.contains(MapCoord(targetTestCoord.mCol, targetTestCoord.mRow + 1))) return true;
    }
    if (currentCoordHasBotNeighbor && targetCoordHasTopNeighbor)
    {
        auto currentCoordBotNeighborLinksIter = generatedLinks.find(MapCoord(currentCoord.mCol, currentCoord.mRow + 1));
        if (currentCoordBotNeighborLinksIter != generatedLinks.cend() && currentCoordBotNeighborLinksIter->second.contains(MapCoord(targetTestCoord.mCol, targetTestCoord.mRow - 1))) return true;
    }
    
    return false;
}

glm::vec3 GenerateNodePositionFromCoord(const MapCoord& mapCoord)
{
    // Base calculation
    glm::vec3 result = glm::vec3
    (
        MAP_MIN_BOUNDS.x + 4.0f * mapCoord.mCol, // Base horizontal spacing
        MAP_MIN_BOUNDS.y + 10.0f - mapCoord.mRow * 5.0f + mapCoord.mCol * 2.0f, // Base vertical alignment + staircase increment
        0.0f
     );
    
    // Add noise
    result.x += math::RandomFloat(-1.0f, 1.0f);
    result.y += math::RandomFloat(-1.0f, 1.0f);
    
    return result;
}

MapCoord SelectNextMapCoord(const MapCoord& currentMapCoord, const int currentCol, const int mapCols, const int mapRows)
{
    auto randRow = math::Max(math::Min(mapRows - 1, currentMapCoord.mRow + math::RandomInt(-1, 1)), 0);
    return currentCol == mapCols - 2 ? MapCoord(mapCols - 1, mapRows/2) : MapCoord(currentCol + 1, randRow);
}

void GenerateMap(const bool forceOneStartingNode, std::map<MapCoord, glm::vec3>& generatedPositions, std::map<MapCoord, std::unordered_set<MapCoord, MapCoordHasher>>& generatedLinks)
{
    const int iterations = 4;
    const int mapCols = 10;
    const int mapRows = 5;
    
//    int map[mapRows][mapCols] =
//    {
//        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//    };
    
    for (int i = 0; i < iterations; ++i)
    {
        auto currentCoordinate = forceOneStartingNode ? MapCoord(0, mapRows/2) : MapCoord(0, math::RandomInt(0, mapRows - 1));
        generatedPositions[currentCoordinate] = GenerateNodePositionFromCoord(currentCoordinate);
        
        for (int col = 1; col < mapCols - 1; ++col)
        {
            MapCoord targetCoord = SelectNextMapCoord(currentCoordinate, col, mapCols, mapRows);
            
            while (DetectedCrossedEdge(currentCoordinate, targetCoord, generatedLinks, mapRows))
            {
                targetCoord = SelectNextMapCoord(currentCoordinate, col, mapCols, mapRows);
            }
            
            generatedLinks[currentCoordinate].insert(targetCoord);
            currentCoordinate = targetCoord;
            generatedPositions[currentCoordinate] = GenerateNodePositionFromCoord(currentCoordinate);
        }
    }
}

MapUpdater::MapUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
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
    
    std::map<MapCoord, glm::vec3> generatedPositions;
    std::map<MapCoord, std::unordered_set<MapCoord, MapCoordHasher>> generatedLinks;
    
    GenerateMap(false, generatedPositions, generatedLinks);
    
    auto positionCounter = 0;
    for (const auto& positionEntry: generatedPositions)
    {
        SceneObject starSO;

        starSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + (math::RandomSign() == 1 ? "octo_star.bmp" : "quad_star.bmp")), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.5f), false);
        starSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        starSO.mName = strutils::StringId("star_" + std::to_string(positionCounter));
        starSO.mScale.x = 1.5f;
        starSO.mScale.y = 1.5f;
        starSO.mPosition = positionEntry.second;
        starSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(starSO));
        positionCounter++;
    }
        

    for (const auto& linkEntry: generatedLinks)
    {
        for (const auto& targetCoord: linkEntry.second)
        {
            glm::vec3 dirToNext = generatedPositions[targetCoord] - generatedPositions[linkEntry.first];
            auto pathSegments = 2 * static_cast<int>(glm::length(dirToNext));
            
            for (int i = 0; i < pathSegments; ++i)
            {
                SceneObject pathSO;
                
                pathSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "star_path.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), pathSO.mScale, false);
                pathSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                pathSO.mPosition = generatedPositions[linkEntry.first] + dirToNext * (i/static_cast<float>(pathSegments));
                pathSO.mScale = glm::vec3(0.25f, 0.25f, 1.0f);
                pathSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
                mScene.AddSceneObject(std::move(pathSO));
            }
        }
    }

    auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
    worldCamera.SetPosition(glm::vec3(MAP_MIN_BOUNDS.x, MAP_MIN_BOUNDS.y, 0.0f));
}

///------------------------------------------------------------------------------------------------

void MapUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    static glm::vec3 originTouchPos;
    static glm::vec3 cameraVelocity;
    static float previousPinchDistance = 0.0f;
    
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return;
    
    auto& inputContext = GameSingletons::GetInputContext();
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    
    auto camPos = worldCamera.GetPosition();
    auto camZoom = worldCamera.GetZoomFactor();
    
    // Touch Position and Map velocity reset on FingerDown
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        originTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        cameraVelocity = glm::vec3(0.0f);
    }
    // Map Position/Zoom flow on FingerMotion
    else if (inputContext.mEventType == SDL_FINGERMOTION)
    {
        // Pinch zoom flow
        if (GameSingletons::GetInputContext().mPinchDistance > 0.0f && previousPinchDistance > 0.0f && GameSingletons::GetInputContext().mMultiGestureActive)
        {
            camZoom = worldCamera.GetZoomFactor() + dtMillis * (GameSingletons::GetInputContext().mPinchDistance - previousPinchDistance) * CAMERA_ZOOM_SPEED;
        }
        // Pan flow
        else if (glm::length(originTouchPos) != 0.0f && GameSingletons::GetInputContext().mMultiGestureActive == false)
        {
            const auto deltaMotion = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix()) - originTouchPos;
            
            if (glm::length(deltaMotion) < MAX_MAP_VELOCITY_LENGTH)
            {
                cameraVelocity = deltaMotion;
            }
        }
    }
    // Reset touch pos on FingerUp
    else if (inputContext.mEventType == SDL_FINGERUP)
    {
         originTouchPos = glm::vec3(0.0f);
    };
    
    // Calculate potential new camera position
    if (glm::length(cameraVelocity) > MIN_CAMERA_VELOCITY_TO_START_MOVEMENT)
    {
        camPos = worldCamera.GetPosition() - cameraVelocity * dtMillis * MAP_VELOCITY_INTEGRATION_SPEED;
        cameraVelocity.x *= MAP_VELOCITY_DAMPING;
        cameraVelocity.y *= MAP_VELOCITY_DAMPING;
    }
    else
    {
        cameraVelocity = glm::vec3(0.0f);
    }
    
    // Clamp and apply camera position
    camPos.x = math::Max(math::Min(MAP_MAX_BOUNDS.x, camPos.x), MAP_MIN_BOUNDS.x);
    camPos.y = math::Max(math::Min(MAP_MAX_BOUNDS.y, camPos.y), MAP_MIN_BOUNDS.y);
    worldCamera.SetPosition(camPos);
    
    // Clamp and apply camera zoom
    camZoom = math::Max(math::Min(CAMERA_MAX_ZOOM_FACTOR, camZoom), Camera::DEFAULT_CAMERA_ZOOM_FACTOR);
    worldCamera.SetZoomFactor(camZoom);
    
    // Keep track of previous finger pinch distance
    previousPinchDistance = GameSingletons::GetInputContext().mPinchDistance;
    
    // Animate all SOs
    for (auto& sceneObject: sceneObjects)
    {
        if (sceneObject.mAnimation)
        {
            sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
        }
    }
}

///------------------------------------------------------------------------------------------------

void MapUpdater::OnAppStateChange(Uint32 event)
{
    static bool hasLeftForegroundOnce = false;
    
    switch (event)
    {
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND:
        {
#ifdef DEBUG
            hasLeftForegroundOnce = true;
#endif
        } break;
            
        case SDL_APP_WILLENTERFOREGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
        {
#ifdef DEBUG
            if (hasLeftForegroundOnce)
            {
                OpenDebugConsole();
            }
#endif
        } break;
    }
}

///------------------------------------------------------------------------------------------------

std::string MapUpdater::GetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void MapUpdater::OpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------
