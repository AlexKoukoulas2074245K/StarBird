///------------------------------------------------------------------------------------------------
///  StatsUpgradeUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "StatsUpgradeUpdater.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

static const strutils::StringId VESSEL_SCENE_OBJECT_NAME = strutils::StringId("VESSEL");

static const char* LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME = "left_navigation_arrow_mm.bmp";

static const glm::vec3 LAB_BACKGROUND_POS = glm::vec3(-1.8f, 0.0f, -1.0f);
static const glm::vec3 LAB_BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);

static const glm::vec3 NAVIGATION_ARROW_SCALE = glm::vec3(3.0f, 2.0f, 0.0f);
static const glm::vec3 NAVIGATION_ARROW_POSITION = glm::vec3(-4.0f, 10.0f, 0.0f);

static const glm::vec3 VESSEL_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 VESSEL_SCALE = glm::vec3(7.0f, 7.0f, 1.0f);

static const float NAVIGATION_ARROW_PULSING_SPEED = 0.01f;
static const float NAVIGATION_ARROW_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;

///------------------------------------------------------------------------------------------------

StatsUpgradeUpdater::StatsUpgradeUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mSelectionState(SelectionState::NO_STATS_SELECTED)
    
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif

    CreateSceneObjects();
}

///------------------------------------------------------------------------------------------------

StatsUpgradeUpdater::~StatsUpgradeUpdater()
{
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective StatsUpgradeUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return PostStateUpdateDirective::BLOCK_UPDATE;
    
    switch (mSelectionState)
    {
        case SelectionState::NO_STATS_SELECTED:
        {
            auto& inputContext = GameSingletons::GetInputContext();
            auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
            auto& worldCamera = camOpt->get();
            glm::vec3 originalFingerDownTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
            auto navigationArrowSoOpt = mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME);
            if (inputContext.mEventType == SDL_FINGERDOWN && navigationArrowSoOpt && scene_object_utils::IsPointInsideSceneObject(navigationArrowSoOpt->get(), originalFingerDownTouchPos))
            {
                mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::LAB, "", true));
                mSelectionState = SelectionState::TRANSITIONING_TO_NEXT_SCREEN;
            }
        } break;
            
        case SelectionState::TRANSITIONING_TO_NEXT_SCREEN:
        {
            return PostStateUpdateDirective::BLOCK_UPDATE;
        } break;
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

void StatsUpgradeUpdater::VOnAppStateChange(Uint32 event)
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

std::string StatsUpgradeUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void StatsUpgradeUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void StatsUpgradeUpdater::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = LAB_BACKGROUND_SCALE;
        bgSO.mPosition = LAB_BACKGROUND_POS;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // Navigation arrow
    {
        SceneObject arrowSo;
        arrowSo.mPosition = NAVIGATION_ARROW_POSITION;
        arrowSo.mScale = NAVIGATION_ARROW_SCALE;
        arrowSo.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, NAVIGATION_ARROW_PULSING_SPEED, NAVIGATION_ARROW_PULSING_ENLARGEMENT_FACTOR, false);
        arrowSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        arrowSo.mName = game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME;
        arrowSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(arrowSo));
    }
    
    // Vessel
    {
        auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        const auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject vesselSo;
        vesselSo.mPosition = VESSEL_POSITION;
        vesselSo.mScale = VESSEL_SCALE;
        vesselSo.mAnimation = playerObjectDef.mAnimations.at(game_constants::DEFAULT_SCENE_OBJECT_STATE)->VClone();
        
        vesselSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        vesselSo.mName = VESSEL_SCENE_OBJECT_NAME;
        vesselSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(vesselSo));
    }
}

///------------------------------------------------------------------------------------------------
