///------------------------------------------------------------------------------------------------
///  MapUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "FullScreenOverlayController.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "MapUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

static const std::unordered_map<Map::NodeType, Scene::SceneType> NODE_TYPE_TO_SCENE_TYPE =
{
    { Map::NodeType::NORMAL_ENCOUNTER, Scene::SceneType::LEVEL },
    { Map::NodeType::HARD_ENCOUNTER, Scene::SceneType::LEVEL },
    { Map::NodeType::BOSS_ENCOUNTER, Scene::SceneType::LEVEL },
    { Map::NodeType::LAB, Scene::SceneType::LAB }
};

static const float MAX_MAP_VELOCITY_LENGTH = 5.0f;
static const float MAP_VELOCITY_DAMPING = 0.9f;
static const float MAP_VELOCITY_INTEGRATION_SPEED = 0.04f;
static const float CAMERA_MAX_ZOOM_FACTOR = 2.4f;
static const float CAMERA_MIN_ZOOM_FACTOR = 0.4f;
static const float CAMERA_ZOOM_SPEED = 0.1f;
static const float MIN_CAMERA_VELOCITY_TO_START_MOVEMENT = 0.0001f;
static const float MAP_NODE_CLICK_BIAS = 3.0f;

///------------------------------------------------------------------------------------------------

MapUpdater::MapUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mMap(scene, GameSingletons::GetMapData(), glm::ivec2(9, 5), GameSingletons::GetCurrentMapCoord(), true)
    , mSelectedMapCoord(0, 0)
    , mTransitioning(false)
    
{
    if (GameSingletons::GetMapData().empty())
    {
        GameSingletons::SetMapData(mMap.GetMapData());
    }
    
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif

    auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
    
    // Center camera to the midpoint of all available nodes
    glm::vec3 positionAccum = mMap.GetMapData().at(GameSingletons::GetCurrentMapCoord()).mPosition;
    const auto& activeCoords = mMap.GetMapData().at(GameSingletons::GetCurrentMapCoord()).mNodeLinks;
    for (auto& linkedCoord: activeCoords)
    {
        positionAccum += mMap.GetMapData().at(linkedCoord).mPosition;
    }
    
    worldCamera.SetPosition(glm::vec3(positionAccum.x/(activeCoords.size() + 1), positionAccum.y/(activeCoords.size() + 1), 0.0f));
}

///------------------------------------------------------------------------------------------------

MapUpdater::~MapUpdater()
{
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective MapUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mTransitioning)
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    static glm::vec3 originTouchPos;
    static glm::vec3 cameraVelocity;
    static float previousPinchDistance = 0.0f;
    
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return PostStateUpdateDirective::BLOCK_UPDATE;
    
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
        
        if (SelectedActiveLevel(originTouchPos))
        {
            GameSingletons::SetCurrentMapCoord(mSelectedMapCoord);
            mTransitioning = true;
            
            assert(NODE_TYPE_TO_SCENE_TYPE.contains(mMap.GetMapData().at(mSelectedMapCoord).mNodeType));
            auto nextSceneType = NODE_TYPE_TO_SCENE_TYPE.at(mMap.GetMapData().at(mSelectedMapCoord).mNodeType);
            
            mScene.ChangeScene(Scene::TransitionParameters(nextSceneType, nextSceneType == Scene::SceneType::LEVEL ? "test_level" : "", true));
            
            return PostStateUpdateDirective::BLOCK_UPDATE;
        }
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
        camPos = worldCamera.GetPosition() - cameraVelocity * dtMillis * MAP_VELOCITY_INTEGRATION_SPEED *  1.0f/worldCamera.GetZoomFactor();
        cameraVelocity.x *= MAP_VELOCITY_DAMPING;
        cameraVelocity.y *= MAP_VELOCITY_DAMPING;
    }
    else
    {
        cameraVelocity = glm::vec3(0.0f);
    }
    
    // Clamp and apply camera position
    camPos.x = math::Max(math::Min(game_constants::MAP_MAX_WORLD_BOUNDS.x, camPos.x), game_constants::MAP_MIN_WORLD_BOUNDS.x);
    camPos.y = math::Max(math::Min(game_constants::MAP_MAX_WORLD_BOUNDS.y, camPos.y), game_constants::MAP_MIN_WORLD_BOUNDS.y);
    worldCamera.SetPosition(camPos);
    
    // Clamp and apply camera zoom
    camZoom = math::Max(math::Min(CAMERA_MAX_ZOOM_FACTOR, camZoom), CAMERA_MIN_ZOOM_FACTOR);
    worldCamera.SetZoomFactor(camZoom);
    
    // Keep track of previous finger pinch distance
    previousPinchDistance = GameSingletons::GetInputContext().mPinchDistance;
    
    // Animate all SOs
    for (auto& sceneObject: sceneObjects)
    {
        if (sceneObject.mAnimation && !sceneObject.mAnimation->VIsPaused())
        {
            sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
        }
        
        for (auto& extraAnimation: sceneObject.mExtraCompoundingAnimations)
        {
            if (!extraAnimation->VIsPaused())
            {
                extraAnimation->VUpdate(dtMillis, sceneObject);
            }
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void MapUpdater::VOnAppStateChange(Uint32 event)
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
                VOpenDebugConsole();
            }
#endif
        } break;
    }
}

///------------------------------------------------------------------------------------------------

std::string MapUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void MapUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

bool MapUpdater::SelectedActiveLevel(const glm::vec3& touchPos)
{
    const auto& currentMapCoord = GameSingletons::GetCurrentMapCoord();
    for (const auto& linkedMapCoord: mMap.GetMapData().at(currentMapCoord).mNodeLinks)
    {
        if (scene_object_utils::IsPointInsideSceneObject(mScene.GetSceneObject(strutils::StringId(linkedMapCoord.ToString()))->get(), glm::vec2(touchPos.x, touchPos.y), glm::vec2(MAP_NODE_CLICK_BIAS)))
        {
            mSelectedMapCoord = linkedMapCoord;
            return true;
        }
    }
    
    return false;
}

///------------------------------------------------------------------------------------------------
