///------------------------------------------------------------------------------------------------
///  LabUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "LabUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

LabUpdater::LabUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mTransitioning(false)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif

    auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
    worldCamera.SetPosition(glm::vec3(0.0f));
    
    CreateSceneObjects();
}

///------------------------------------------------------------------------------------------------

LabUpdater::~LabUpdater()
{
}

///------------------------------------------------------------------------------------------------

void LabUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    mStateMachine.Update(dtMillis);
    
    if (mTransitioning) return;
    
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    
    // Touch Position and Map velocity reset on FingerDown
    auto& inputContext = GameSingletons::GetInputContext();
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        glm::vec3 touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        if (scene_object_utils::IsPointInsideSceneObject(mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME)->get(), glm::vec2(touchPos.x, touchPos.y), glm::vec2(game_constants::MAP_NODE_CLICK_BIAS)))
        {
            mTransitioning = true;
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            return;
        }
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
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnAppStateChange(Uint32 event)
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

std::string LabUpdater::GetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void LabUpdater::OpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void LabUpdater::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_constants::LAB_BACKGROUND_SCALE;
        bgSO.mPosition = game_constants::LAB_BACKGROUND_POS;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // Navigation arrow
    {
        SceneObject bgSO;
        bgSO.mScale = glm::vec3(3.0f, 2.0f, 0.0f);
        bgSO.mPosition = glm::vec3(-4.0f, 10.0f, 0.0f);
        bgSO.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), 0.0f, game_constants::MAP_STAR_PATH_PULSING_SPEED, game_constants::MAP_STAR_PATH_PULSING_ENLARGEMENT_FACTOR, false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // Options
    for (int i = 0; i < 4; ++i)
    {
        SceneObject labOptionSO;
        labOptionSO.mScale = glm::vec3(4.0f, 4.0f, 1.0f);
        if (i == 0 || i == 2)
        {
            labOptionSO.mPosition.x -= labOptionSO.mScale.x;
        }
        if (i == 1 || i == 3)
        {
            labOptionSO.mPosition.x += labOptionSO.mScale.x;
        }
        if (i == 0 || i == 1)
        {
            labOptionSO.mPosition.y += labOptionSO.mScale.x;
        }
        if (i == 2 || i == 3)
        {
            labOptionSO.mPosition.y -= labOptionSO.mScale.x;
        }
        labOptionSO.mPosition.z = game_constants::LAB_OPTIONS_Z;
        labOptionSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_OPTIONS_TEXTURE_FILE_NAME_PREFIX + std::to_string(i + 1) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        labOptionSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        labOptionSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        labOptionSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(labOptionSO));
    }
}

///------------------------------------------------------------------------------------------------
