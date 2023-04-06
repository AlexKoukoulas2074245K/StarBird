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
#include "FontRepository.h"
#include "SceneObjectUtils.h"
#include "StatsUpgradeUpdater.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId VESSEL_SCENE_OBJECT_NAME = strutils::StringId("VESSEL");
static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT");

static const char* CONFIRMATION_BUTTON_TEXTURE_FILE_NAME = "confirmation_button_mm.bmp";
static const char* LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME = "left_navigation_arrow_mm.bmp";

static const glm::vec3 BACKGROUND_POS = glm::vec3(-1.8f, 0.0f, -1.0f);
static const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);

static const glm::vec3 NAVIGATION_ARROW_SCALE = glm::vec3(3.0f, 2.0f, 0.0f);
static const glm::vec3 NAVIGATION_ARROW_POSITION = glm::vec3(-4.0f, 10.0f, 0.0f);

static const glm::vec3 VESSEL_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 VESSEL_SCALE = glm::vec3(7.0f, 7.0f, 1.0f);

static const float NAVIGATION_ARROW_PULSING_SPEED = 0.01f;
static const float NAVIGATION_ARROW_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;

static const glm::vec3 CONFIRMATION_BUTTON_POSITION = glm::vec3(0.0f, -6.0f, 0.0f);
static const glm::vec3 CONFIRMATION_BUTTON_SCALE = glm::vec3(3.5f, 3.5f, 0.0f);

static const glm::vec3 CONFIRMATION_BUTTON_TEXT_POSITION = glm::vec3(-0.8f, -6.3f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_SCALE = glm::vec3(0.007f, 0.007f, 1.0f);

static const glm::vec3 TEXT_PROMPT_POSITION = glm::vec3(0.0f, 7.2f, 0.5f);
static const glm::vec3 TEXT_PROMPT_SCALE = glm::vec3(12.0f, 10.0f, 1.0f);

static const float CONFIRMATION_BUTTON_PULSING_SPEED = 0.01f;
static const float CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;
static const float CONFIRMATION_BUTTON_ROTATION_SPEED = 0.0002f;

static const float DROPPED_CRYSTAL_SPEED = 0.0009f;
static const float DROPPED_CRYSTAL_DISTANCE_FACTOR = 24.0f;
static const float DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG = 0.5f;
static const float DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG = 2.0f;

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
    
    auto currentTotalCost = 0;
    for (auto& statControllerEntry: mStatControllers)
    {
        currentTotalCost += statControllerEntry.second->GetCurrentCost();
    }
    
    for (auto& statControllerEntry: mStatControllers)
    {
        statControllerEntry.second->Update(dtMillis, currentTotalCost);
    }
    
    if (mSelectionState != SelectionState::TRANSITIONING_TO_NEXT_SCREEN && mSelectionState != SelectionState::EXPENDING_CRYSTALS)
    {
        mSelectionState = currentTotalCost == 0 ? SelectionState::NO_STATS_SELECTED : SelectionState::ONE_OR_MORE_STATS_HAVE_BEEN_SELECTED;
        
        auto navigationArrowSoOpt = mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME);
        if (navigationArrowSoOpt)
        {
            auto& navigationArrowSo = navigationArrowSoOpt->get();
            if (currentTotalCost == 0)
            {
                navigationArrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (navigationArrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    navigationArrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            else
            {
                navigationArrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (navigationArrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
                {
                    navigationArrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                }
            }
        }
        
        auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
        if (confirmationButtonSoOpt)
        {
            auto& confirmationButtonSo = confirmationButtonSoOpt->get();
            
            if (currentTotalCost != 0)
            {
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            else
            {
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
                {
                    confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                }
            }
        }
        
        // & text
        auto confirmationButtonTextSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
        if (confirmationButtonTextSoOpt)
        {
            auto& confirmationButtonTextSo = confirmationButtonTextSoOpt->get();
            
            if (currentTotalCost != 0)
            {
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            else
            {
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
                {
                    confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                }
            }
        }
    }
    
    auto& inputContext = GameSingletons::GetInputContext();
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    glm::vec3 originalFingerDownTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
    
    switch (mSelectionState)
    {
        case SelectionState::NO_STATS_SELECTED:
        {
            auto navigationArrowSoOpt = mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME);
            if (inputContext.mEventType == SDL_FINGERDOWN && navigationArrowSoOpt && scene_object_utils::IsPointInsideSceneObject(navigationArrowSoOpt->get(), originalFingerDownTouchPos))
            {
                mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::LAB, "", true));
                mSelectionState = SelectionState::TRANSITIONING_TO_NEXT_SCREEN;
            }
        } break;
          
        case SelectionState::ONE_OR_MORE_STATS_HAVE_BEEN_SELECTED:
        {
            auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
            if (inputContext.mEventType == SDL_FINGERDOWN && confirmationButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(confirmationButtonSoOpt->get(), originalFingerDownTouchPos))
            {
                OnConfirmationButtonPressed();
                
                for (const auto& statControllerEntry: mStatControllers)
                {
                    switch (statControllerEntry.first)
                    {
                        case StatType::ATTACK_STAT:
                        {
                            GameSingletons::SetPlayerAttackStat(statControllerEntry.second->GetCurrentStatValue());
                        } break;
                            
                        case StatType::HASTE_STAT:
                        {
                            GameSingletons::SetPlayerBulletSpeedStat(statControllerEntry.second->GetCurrentStatValue());
                        } break;
                            
                        case StatType::SPEED_STAT:
                        {
                            GameSingletons::SetPlayerMovementSpeedStat(statControllerEntry.second->GetCurrentStatValue());
                        } break;
                        
                        case StatType::HEALTH_STAT:
                        {
                            GameSingletons::SetPlayerMaxHealth(statControllerEntry.second->GetCurrentStatValue());
                        } break;
                    }
                    
                    GameSingletons::SetCrystalCount(GameSingletons::GetCrystalCount() - statControllerEntry.second->GetCurrentCost());
                    CreateCrystalsTowardTargetPosition(statControllerEntry.second->GetCurrentCost(), statControllerEntry.second->GetTargetCrystalPosition());
                }
                
                mSelectionState = SelectionState::EXPENDING_CRYSTALS;
            }
        } break;
            
        case SelectionState::EXPENDING_CRYSTALS:
        {
            auto crystalNameIter = mCrystalSceneObjectNames.begin();
            
            while (crystalNameIter != mCrystalSceneObjectNames.end())
            {
                auto crystalSoOpt = mScene.GetSceneObject(*crystalNameIter);
                if (crystalSoOpt && crystalSoOpt->get().mAnimation->VIsPaused())
                {
                    mScene.RemoveAllSceneObjectsWithName(*crystalNameIter);
                    crystalNameIter = mCrystalSceneObjectNames.erase(crystalNameIter);
                }
                else
                {
                    crystalNameIter++;
                }
            }
            
            if (mCrystalSceneObjectNames.empty())
            {
                mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
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

strutils::StringId StatsUpgradeUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
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
        bgSO.mScale = BACKGROUND_SCALE;
        bgSO.mPosition = BACKGROUND_POS;
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
        arrowSo.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, NAVIGATION_ARROW_PULSING_SPEED, NAVIGATION_ARROW_PULSING_ENLARGEMENT_FACTOR, false);
        arrowSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        arrowSo.mName = game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME;
        arrowSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        arrowSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
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
    
    // Confirmation button background
    {
        SceneObject confirmationButtonSo;
        confirmationButtonSo.mPosition = CONFIRMATION_BUTTON_POSITION;
        confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
        confirmationButtonSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CONFIRMATION_BUTTON_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Z, 0.0f, CONFIRMATION_BUTTON_ROTATION_SPEED, false);
        confirmationButtonSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        confirmationButtonSo.mName = CONFIRMATION_BUTTON_NAME;
        confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        confirmationButtonSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(confirmationButtonSo));
    }
    
    // Confirmation button text
    {
        SceneObject confirmationButtonTextSo;
        confirmationButtonTextSo.mPosition = CONFIRMATION_BUTTON_TEXT_POSITION;
        confirmationButtonTextSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        confirmationButtonTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        confirmationButtonTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        confirmationButtonTextSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        confirmationButtonTextSo.mName = CONFIRMATION_BUTTON_TEXT_NAME;
        confirmationButtonTextSo.mText = "Select";
        confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mScene.AddSceneObject(std::move(confirmationButtonTextSo));
    }
    
    {
        mStatControllers[StatType::ATTACK_STAT] = std::make_unique<StatUpgradeAreaController>(mScene, std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "diagonal_upgrade_area_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(6.22f, 4.8f, 1.0f), false), glm::vec3(2.93f, 7.32f, 0.5f), glm::vec3(6.22f, 4.8f, 1.0f), "ATTACK ", GameSingletons::GetPlayerAttackStat(), 1.0f, false);
        
        
        mStatControllers[StatType::HASTE_STAT] = std::make_unique<StatUpgradeAreaController>(mScene, std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "diagonal_upgrade_area_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(6.22f, 4.8f, 1.0f), false), glm::vec3(2.93f, -2.32f, 0.5f), glm::vec3(6.22f, 4.8f, 1.0f), "HASTE ", GameSingletons::GetPlayerBulletSpeedStat(), 0.1f, true);
    }
}

///------------------------------------------------------------------------------------------------

void StatsUpgradeUpdater::CreateCrystalsTowardTargetPosition(const int crystalCount, const glm::vec3& position)
{
    for (int i = 0; i < crystalCount; ++i)
    {
        auto& resService = resources::ResourceLoadingService::GetInstance();
        
        SceneObject crystalSo;
        
        glm::vec3 firstControlPoint(game_constants::GUI_CRYSTAL_POSITION);
        glm::vec3 thirdControlPoint(position);
        glm::vec3 secondControlPoint((thirdControlPoint + firstControlPoint) * 0.5f + glm::vec3(math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), 0.0f));
        
        firstControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
        secondControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
        thirdControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
        
        float speedNoise = math::RandomFloat(-DROPPED_CRYSTAL_SPEED/5, DROPPED_CRYSTAL_SPEED/5);
        float speedMultiplier = DROPPED_CRYSTAL_DISTANCE_FACTOR/glm::distance(thirdControlPoint, game_constants::GUI_CRYSTAL_POSITION);
        
        const strutils::StringId droppedCrystalName = strutils::StringId(std::to_string(SDL_GetPerformanceCounter()));
        mCrystalSceneObjectNames.push_back(droppedCrystalName);
        
        crystalSo.mAnimation = std::make_unique<BezierCurvePathAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), math::BezierCurve({firstControlPoint, secondControlPoint, thirdControlPoint}), (DROPPED_CRYSTAL_SPEED + speedNoise) * speedMultiplier, false);
       
        crystalSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false));
        
        crystalSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalSo.mPosition = firstControlPoint;
        crystalSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
        crystalSo.mName = droppedCrystalName;
        mScene.AddSceneObject(std::move(crystalSo));
    }
}

///------------------------------------------------------------------------------------------------

void StatsUpgradeUpdater::OnConfirmationButtonPressed()
{
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    auto confirmationButtonTexSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
    
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
        
        confirmationButtonSo.mExtraCompoundingAnimations.clear();
        confirmationButtonSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONFIRMATION_BUTTON_PULSING_SPEED * 2, CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR * 10, false));
    }
    
    if (confirmationButtonTexSoOpt)
    {
        auto& confirmationButtonTextSo = confirmationButtonTexSoOpt->get();
        confirmationButtonTextSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        
        confirmationButtonTextSo.mExtraCompoundingAnimations.clear();
        confirmationButtonTextSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonTextSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_TEXT_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONFIRMATION_BUTTON_PULSING_SPEED * 2, CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR / 40, false));
    }
}
