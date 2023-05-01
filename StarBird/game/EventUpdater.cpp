///------------------------------------------------------------------------------------------------
///  EventUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/05/2023
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "EventUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "states/DebugConsoleGameState.h"
#include "dataloaders/GUISceneLoader.h"
#include "datarepos/FontRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

static const glm::vec3 BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, -7.0f);
static const glm::vec3 FULL_SCREEN_OVERLAY_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 FULL_SCREEN_OVERLAY_SCALE = glm::vec3(200.0f, 200.0f, 1.0f);

///------------------------------------------------------------------------------------------------

EventUpdater::EventUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mTransitioning(false)
    
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    CreateSceneObjects();
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective EventUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mTransitioning)
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return PostStateUpdateDirective::BLOCK_UPDATE;
    
    auto& inputContext = GameSingletons::GetInputContext();
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    auto& guiCamera = camOpt->get();
    
    // Touch Position
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
    }
    
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

void EventUpdater::VOnAppStateChange(Uint32 event)
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

std::string EventUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId EventUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void EventUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void EventUpdater::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();

    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_constants::MAP_BACKGROUND_SCALE;
        bgSO.mPosition = BACKGROUND_POSITION;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BACKGROUND_TEXTURE_FILE_PATH + std::to_string(GameSingletons::GetBackgroundIndex()) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mScale = FULL_SCREEN_OVERLAY_SCALE;
        overlaySo.mPosition = FULL_SCREEN_OVERLAY_POSITION;
        overlaySo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.8f;
        mScene.AddSceneObject(std::move(overlaySo));
    }
}

///------------------------------------------------------------------------------------------------
