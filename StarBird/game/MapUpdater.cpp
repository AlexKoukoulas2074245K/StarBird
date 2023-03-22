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

///------------------------------------------------------------------------------------------------

MapUpdater::MapUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mMap(scene, glm::ivec2(10, 5), true)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif

    auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
    worldCamera.SetPosition(glm::vec3(game_object_constants::MAP_MIN_WORLD_BOUNDS.x, game_object_constants::MAP_MIN_WORLD_BOUNDS.y, 0.0f));
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
    camPos.x = math::Max(math::Min(game_object_constants::MAP_MAX_WORLD_BOUNDS.x, camPos.x), game_object_constants::MAP_MIN_WORLD_BOUNDS.x);
    camPos.y = math::Max(math::Min(game_object_constants::MAP_MAX_WORLD_BOUNDS.y, camPos.y), game_object_constants::MAP_MIN_WORLD_BOUNDS.y);
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
