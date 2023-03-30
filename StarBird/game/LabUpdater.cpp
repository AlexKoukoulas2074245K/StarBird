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
    , mCarouselState(CarouselState::STATIONARY)
    , mCarouselRads(0.0f)
    , mCarouselTargetRads(0.0f)
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
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE || mTransitioning)
    {
        return;
    }
    
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    
    // Touch Position and Map velocity reset on FingerDown
    auto& inputContext = GameSingletons::GetInputContext();
    static glm::vec3 touchPos(0.0f);
    static bool exhaustedMove = false;
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        if (scene_object_utils::IsPointInsideSceneObject(mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME)->get(), glm::vec2(touchPos.x, touchPos.y)))
        {
            mTransitioning = true;
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            return;
        }
    }
    else if (inputContext.mEventType == SDL_FINGERMOTION && !exhaustedMove)
    {
        auto currentTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        if (mCarouselState == CarouselState::STATIONARY)
        {
            if (currentTouchPos.x > touchPos.x)
            {
                mCarouselState = CarouselState::MOVING_LEFT;
                mCarouselTargetRads = mCarouselRads + (math::PI * 2.0f / (mLabOptionSoNames.size()));
            }
            else
            {
                mCarouselState = CarouselState::MOVING_RIGHT;
                mCarouselTargetRads = mCarouselRads - (math::PI * 2.0f / (mLabOptionSoNames.size()));
            }
            
            exhaustedMove = true;
        }
    }
    else if (inputContext.mEventType == SDL_FINGERUP)
    {
        exhaustedMove = false;
    }
    
    
    // Rotate lab options
    if (mCarouselState == CarouselState::MOVING_LEFT)
    {
        mCarouselRads += dtMillis * game_constants::MAP_NODE_ROTATION_SPEED * 30.0f;
        if (mCarouselRads >= mCarouselTargetRads)
        {
            mCarouselRads = mCarouselTargetRads;
            mCarouselState = CarouselState::STATIONARY;
        }
    }
    else if (mCarouselState == CarouselState::MOVING_RIGHT)
    {
        mCarouselRads -= dtMillis * game_constants::MAP_NODE_ROTATION_SPEED * 30.0f;
        if (mCarouselRads <= mCarouselTargetRads)
        {
            mCarouselRads = mCarouselTargetRads;
            mCarouselState = CarouselState::STATIONARY;
        }
    }
    
    // Give fake perspective to all Lab options
    for (int i = 0; i < static_cast<int>(mLabOptionSoNames.size()); ++i)
    {
        auto labOptionSoOpt = mScene.GetSceneObject(mLabOptionSoNames[i]);
        if (labOptionSoOpt)
        {
            auto& labOptionSo = labOptionSoOpt->get();
            PositionCarouselObject(labOptionSo, i);
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
        SceneObject arrowSo;
        arrowSo.mPosition = game_constants::LAB_NAVIGATION_ARROW_POSITION;
        arrowSo.mScale = game_constants::LAB_NAVIGATION_ARROW_SCALE;
        arrowSo.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), 0.0f, game_constants::LAB_ARROW_PULSING_SPEED, game_constants::LAB_ARROW_PULSING_ENLARGEMENT_FACTOR, false);
        arrowSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        arrowSo.mName = game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME;
        arrowSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(arrowSo));
    }
    
    // Options
    const int optionCount = 4;
    mLabOptionSoNames.resize(optionCount);
    
    for (int i = 0; i < static_cast<int>(mLabOptionSoNames.size()); ++i)
    {
        SceneObject labOptionSO;
        labOptionSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_OPTIONS_TEXTURE_FILE_NAME_PREFIX + std::to_string(i) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        labOptionSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        labOptionSO.mName = strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i));
        labOptionSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        PositionCarouselObject(labOptionSO, i);
        mLabOptionSoNames[i] = labOptionSO.mName;
        mScene.AddSceneObject(std::move(labOptionSO));
    }
}

///------------------------------------------------------------------------------------------------

void LabUpdater::PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const
{
    float optionRadsOffset = objectIndex * (math::PI * 2.0f / mLabOptionSoNames.size());
    carouselObject.mPosition.x = math::Sinf(mCarouselRads + optionRadsOffset) * game_constants::LAB_CAROUSEL_OBJECT_X_MULTIPLIER;
    carouselObject.mPosition.z = game_constants::LAB_OPTIONS_Z + math::Cosf(mCarouselRads + optionRadsOffset);
    carouselObject.mScale = glm::vec3(carouselObject.mPosition.z + game_constants::LAB_CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT, carouselObject.mPosition.z + game_constants::LAB_CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT, 1.0f);
}

///------------------------------------------------------------------------------------------------
