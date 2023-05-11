///------------------------------------------------------------------------------------------------
///  ResearchUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/04/2023
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "CarouselController.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "ResearchUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "Sounds.h"
#include "states/DebugConsoleGameState.h"
#include "states/SettingsMenuGameState.h"
#include "datarepos/FontRepository.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"
#include "../utils/ObjectiveCUtils.h"

#include <vector>
#include <unordered_map>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_FIRST_LINE_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT_FIRST_LINE");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_SECOND_LINE_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT_SECOND_LINE");
static const strutils::StringId CONFIRMATION_BUTTON_CRYSTAL_ICON_NAME = strutils::StringId("CONFIRMATION_BUTTON_CRYSTAL_ICON_NAME");
static const strutils::StringId UPGRADE_TEXT_NAME = strutils::StringId("UPGRADE_TEXT");
static const strutils::StringId UNLOCK_BAR_NAME = strutils::StringId("UNLOCK_BAR");
static const strutils::StringId UNLOCK_BAR_FRAME_NAME = strutils::StringId("UNLOCK_BAR_FRAME");
static const strutils::StringId UNLOCK_BAR_TEXT_NAME = strutils::StringId("UNLOCK_BAR_TEXT");

static const char* LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME = "left_navigation_arrow_mm.bmp";
static const char* CONFIRMATION_BUTTON_TEXTURE_FILE_NAME = "confirmation_button_mm.bmp";

static const glm::vec3 BACKGROUND_POS = glm::vec3(-1.8f, 0.0f, -1.0f);
static const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);

static const glm::vec3 CONFIRMATION_BUTTON_POSITION = glm::vec3(0.0f, -8.0f, 0.0f);
static const glm::vec3 CONFIRMATION_BUTTON_SCALE = glm::vec3(3.5f, 3.5f, 0.0f);

static const glm::vec3 CONFIRMATION_BUTTON_TEXT_FIRST_LINE_POSITION = glm::vec3(-0.74f, -7.9f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_SECOND_LINE_POSITION = glm::vec3(-0.5f, -8.7f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_CRYSTAL_ICON_POSITION = glm::vec3(-0.1f, -8.34, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_SCALE = glm::vec3(0.007f, 0.007f, 1.0f);
static const glm::vec3 CONFIRMATION_BUTTON_CRYSTAL_ICON_SCALE = glm::vec3(0.3f);

static const glm::vec3 UPGRADE_TEXT_POSITION = glm::vec3(0.25f, 4.1f, 0.5f);
static const glm::vec3 UPGRADE_TEXT_SCALE = glm::vec3(0.01f, 0.01f, 1.0f);

inline const glm::vec3 UNLOCK_BAR_POSITION = glm::vec3(0.05f, -3.9f, 0.5f);
inline const glm::vec3 FLYING_CRYSTALS_TARGET_POSITION = glm::vec3(0.0f, -3.9f, 0.5f);
inline const glm::vec3 UNLOCK_BAR_SCALE = glm::vec3(6.5f, 1.4f, 1.0f);

static const glm::vec3 NAVIGATION_ARROW_SCALE = glm::vec3(3.0f, 2.0f, 0.0f);
static const glm::vec3 NAVIGATION_ARROW_POSITION = glm::vec3(-4.0f, 10.0f, 0.0f);

static const float ARROW_PULSING_SPEED = 0.01f;
static const float ARROW_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;
static const float CONFIRMATION_BUTTON_ROTATION_SPEED = 0.0002f;
static const float NAVIGATION_ARROW_PULSING_SPEED = 0.01f;
static const float NAVIGATION_ARROW_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;
static const float DROPPED_CRYSTAL_SPEED = 0.0006f;
static const float DROPPED_CRYSTAL_DISTANCE_FACTOR = 24.0f;
static const float DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG = 2.0f;
static const float UNLOCKED_UPGRADE_SHAKE_SPEED_RAMP = 0.002f;
static const float UNLOCKED_UPGRADE_MAX_SHAKE_MAGNITUDE = 10.0f;
static const float UNLOCKED_UPGRADE_SHINE_SPEED = 1.0f/200.0f;

///------------------------------------------------------------------------------------------------

ResearchUpdater::ResearchUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mOptionSelectionState(OptionSelectionState::OPTION_NOT_SELECTED)
    , mSelectedUpgrade()
    , mCurrentOperationCrystalCost(0)
    , mOptionShakeMagnitude(1.0f)
    , mCarouselMoving(false)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    mStateMachine.RegisterState<SettingsMenuGameState>();
    
    CreateSceneObjects();
    OnCarouselStationary();
}

///------------------------------------------------------------------------------------------------

ResearchUpdater::~ResearchUpdater()
{
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective ResearchUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE)
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    switch (mOptionSelectionState)
    {
        case OptionSelectionState::OPTION_NOT_SELECTED:
        {
            auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
            auto& worldCamera = camOpt->get();
            
            
            auto& inputContext = GameSingletons::GetInputContext();
            auto currentTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
            
            if (inputContext.mEventType == SDL_FINGERDOWN)
            {
                auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
                if (confirmationButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(*confirmationButtonSoOpt, currentTouchPos))
                {
                    OnConfirmationButtonPressed();
                    mOptionSelectionState = OptionSelectionState::EXPEND_CRYSTALS;
                    objectiveC_utils::PlaySound(resources::ResourceLoadingService::RES_SOUNDS_ROOT + sounds::WHOOSH_SFX_PATH, false);
                }
                
                auto navigationArrowSoOpt = mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME);
                if (navigationArrowSoOpt && scene_object_utils::IsPointInsideSceneObject(*navigationArrowSoOpt, currentTouchPos))
                {
                    mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::LAB, "", true));
                    mOptionSelectionState = OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN;
                    objectiveC_utils::PlaySound(resources::ResourceLoadingService::RES_SOUNDS_ROOT + sounds::WHOOSH_SFX_PATH, false);
                }
            }
        
            // Update carousel
            mCarouselController->Update(dtMillis);
            
        } break;
            
        case OptionSelectionState::EXPEND_CRYSTALS:
        {
            mSelectedUpgrade = mUpgrades.at(mCarouselController->GetSelectedIndex());
            auto& upgradeDefinition = GameSingletons::GetAvailableUpgrades().at(mCarouselController->GetSelectedIndex());
            
            auto crystalNameIter = mCrystalSceneObjectNames.begin();
            while (crystalNameIter != mCrystalSceneObjectNames.end())
            {
                auto crystalSoOpt = mScene.GetSceneObject(*crystalNameIter);
                if (crystalSoOpt && crystalSoOpt->get().mAnimation->VIsPaused())
                {
                    mScene.RemoveAllSceneObjectsWithName(*crystalNameIter);
                    crystalNameIter = mCrystalSceneObjectNames.erase(crystalNameIter);
                    upgradeDefinition.mCrystalUnlockProgress++;
                }
                else
                {
                    crystalNameIter++;
                }
            }
            
            if (mCrystalSceneObjectNames.empty())
            {
                if (upgradeDefinition.mCrystalUnlockProgress == upgradeDefinition.mDefaultUnlockCost * GameSingletons::GetResearchCostMultiplier())
                {
                    upgradeDefinition.mUnlocked = true;
                    mOptionSelectionState = OptionSelectionState::UNLOCK_SHAKE;
                }
                else
                {
                    if (mFlows.empty())
                    {
                        mFlows.emplace_back(RepeatableFlow([&]()
                        {
                            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
                            mOptionSelectionState = OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN;
                        }, 1000.0f, RepeatableFlow::RepeatPolicy::ONCE));
                    }
                }
            }
        } break;
            
        case OptionSelectionState::UNLOCK_SHAKE:
        {
            mOptionShakeMagnitude += dtMillis * UNLOCKED_UPGRADE_SHAKE_SPEED_RAMP;
            if (mOptionShakeMagnitude > UNLOCKED_UPGRADE_MAX_SHAKE_MAGNITUDE)
            {
                mOptionShakeMagnitude = UNLOCKED_UPGRADE_MAX_SHAKE_MAGNITUDE;
                
                // Reposition upgrade
                mCarouselController->Update(dtMillis);
                
                // Change animation to shine
                auto upgradeSoOpt = mCarouselController->GetSelectedSceneObject();
                if (upgradeSoOpt)
                {
                    auto& upgradeSo = upgradeSoOpt->get();
                    upgradeSo.mAnimation = std::make_unique<ShineAnimation>(&upgradeSo, upgradeSo.mAnimation->VGetCurrentTextureResourceId(), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::UPGRADE_SHINE_EFFECT_TEXTURE_FILE_NAME), upgradeSo.mAnimation->VGetCurrentMeshResourceId(), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::SHINE_SHADER_FILE_NAME), glm::vec3(1.0f), UNLOCKED_UPGRADE_SHINE_SPEED, false);
                    upgradeSo.mAnimation->SetCompletionCallback([&]()
                    {
                        mFlows.emplace_back(RepeatableFlow([&]()
                        {
                            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
                            mOptionSelectionState = OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN;
                        }, 1000.0f, RepeatableFlow::RepeatPolicy::ONCE));
                    });
                }
                
                mOptionSelectionState = OptionSelectionState::UNLOCK_TEXTURE_TRANSITION;
            }
        } break;
            
        case OptionSelectionState::UNLOCK_TEXTURE_TRANSITION:
        {
            
        } break;
        
        case OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN:
        {
            return PostStateUpdateDirective::BLOCK_UPDATE;
        } break;
    }
    
    auto selectedOptionSoOpt = mCarouselController->GetSelectedSceneObject();
    if (selectedOptionSoOpt && !mCarouselMoving)
    {
        mSelectedUpgrade = mUpgrades.at(mCarouselController->GetSelectedIndex());
        const auto& upgradeDefinition = GameSingletons::GetAvailableUpgrades().at(mCarouselController->GetSelectedIndex());
        
        if ((upgradeDefinition.mCrystalUnlockProgress < upgradeDefinition.mDefaultUnlockCost * GameSingletons::GetResearchCostMultiplier() && !upgradeDefinition.mUnlocked) || mOptionSelectionState == OptionSelectionState::UNLOCK_SHAKE)
        {
            auto currentValue = upgradeDefinition.mCrystalUnlockProgress;
            
            float unlockPerc = currentValue/static_cast<float>(upgradeDefinition.mDefaultUnlockCost * GameSingletons::GetResearchCostMultiplier());
            if (unlockPerc > 0.75f)
            {
                float shakeAbsValue = unlockPerc/20.0f;
                
                auto& optionSceneObject = selectedOptionSoOpt->get();
                
                auto randomOffset = glm::vec3(math::RandomFloat(-shakeAbsValue * mOptionShakeMagnitude, shakeAbsValue * mOptionShakeMagnitude), math::RandomFloat(-shakeAbsValue * mOptionShakeMagnitude, shakeAbsValue * mOptionShakeMagnitude), 0.0f);
                
                optionSceneObject.mPosition.x = randomOffset.x;
                optionSceneObject.mPosition.y = randomOffset.y;
            }
        }
    }
    
    UpdateFadeableSceneObjects(dtMillis);
    UpdateUnlockBarSceneObjects(dtMillis);
    
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
    
    // Update flows
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::VOnAppStateChange(Uint32 event)
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

std::string ResearchUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId ResearchUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void ResearchUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void ResearchUpdater::VOpenSettingsMenu()
{
    mStateMachine.PushState(SettingsMenuGameState::STATE_NAME);
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::CreateSceneObjects()
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

    // Options
    std::vector<resources::ResourceId> researchOptionTextures;
    std::unordered_set<size_t> lockedIndices;
    
    for (size_t i = 0; i < GameSingletons::GetAvailableUpgrades().size(); ++i)
    {
        auto& availableUpgrade = GameSingletons::GetAvailableUpgrades().at(i);
        if (!availableUpgrade.mUnlocked)
        {
            lockedIndices.insert(i);
        }
        
        mUpgrades.push_back(availableUpgrade.mUpgradeNameId);
        researchOptionTextures.push_back(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + availableUpgrade.mTextureFileName));
    }
    
    mCarouselController = std::make_unique<CarouselController>(mScene, researchOptionTextures, [&](){ OnCarouselMovementStart(); }, [&](){ OnCarouselStationary(); }, 2.0f, lockedIndices);
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::OnCarouselMovementStart()
{
    mCarouselMoving = true;
    for (const auto& fadeableSceneObjectName: mFadeableSceneObjects)
    {
        mScene.RemoveAllSceneObjectsWithName(fadeableSceneObjectName);
    }
    
    mFadeableSceneObjects.clear();
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::OnCarouselStationary()
{
    mCarouselMoving = false;
    mSelectedUpgrade = mUpgrades.at(mCarouselController->GetSelectedIndex());
    const auto& upgradeDefinition = GameSingletons::GetAvailableUpgrades().at(mCarouselController->GetSelectedIndex());
    
    mCurrentOperationCrystalCost = math::Min((upgradeDefinition.mDefaultUnlockCost * GameSingletons::GetResearchCostMultiplier()) - upgradeDefinition.mCrystalUnlockProgress, GameSingletons::GetCrystalCount());
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    if (!upgradeDefinition.mUnlocked)
    {
        // Recreate confirmation button
        SceneObject confirmationButtonSo;
        confirmationButtonSo.mPosition = CONFIRMATION_BUTTON_POSITION;
        confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
        confirmationButtonSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CONFIRMATION_BUTTON_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Z, 0.0f, CONFIRMATION_BUTTON_ROTATION_SPEED, false);
        confirmationButtonSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        confirmationButtonSo.mName = CONFIRMATION_BUTTON_NAME;
        confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        confirmationButtonSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mFadeableSceneObjects.push_back(CONFIRMATION_BUTTON_NAME);
        mScene.AddSceneObject(std::move(confirmationButtonSo));
        
        // Confirmation button text top line
        SceneObject confirmationButtonTexFirstLineSo;
        confirmationButtonTexFirstLineSo.mPosition = CONFIRMATION_BUTTON_TEXT_FIRST_LINE_POSITION;
        confirmationButtonTexFirstLineSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        confirmationButtonTexFirstLineSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        confirmationButtonTexFirstLineSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        confirmationButtonTexFirstLineSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        confirmationButtonTexFirstLineSo.mName = CONFIRMATION_BUTTON_TEXT_FIRST_LINE_NAME;
        confirmationButtonTexFirstLineSo.mText = "Spend";
        confirmationButtonTexFirstLineSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mFadeableSceneObjects.push_back(CONFIRMATION_BUTTON_TEXT_FIRST_LINE_NAME);
        mScene.AddSceneObject(std::move(confirmationButtonTexFirstLineSo));
        
        // Confirmation Button text bottom line
        SceneObject confirmationButtonTexSecondLineSo;
        confirmationButtonTexSecondLineSo.mPosition = CONFIRMATION_BUTTON_TEXT_SECOND_LINE_POSITION;
        confirmationButtonTexSecondLineSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        confirmationButtonTexSecondLineSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        confirmationButtonTexSecondLineSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        confirmationButtonTexSecondLineSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        confirmationButtonTexSecondLineSo.mName = CONFIRMATION_BUTTON_TEXT_SECOND_LINE_NAME;
        confirmationButtonTexSecondLineSo.mText = std::to_string(mCurrentOperationCrystalCost);
        confirmationButtonTexSecondLineSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mFadeableSceneObjects.push_back(CONFIRMATION_BUTTON_TEXT_SECOND_LINE_NAME);
        mScene.AddSceneObject(std::move(confirmationButtonTexSecondLineSo));
        
        // Crystal Icon
        SceneObject crystalIconSo;
        crystalIconSo.mPosition = CONFIRMATION_BUTTON_CRYSTAL_ICON_POSITION;
        crystalIconSo.mPosition.x += 0.35f * std::to_string(mCurrentOperationCrystalCost).size();
        crystalIconSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        crystalIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalIconSo.mScale = CONFIRMATION_BUTTON_CRYSTAL_ICON_SCALE;
        crystalIconSo.mName = CONFIRMATION_BUTTON_CRYSTAL_ICON_NAME;
        crystalIconSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mFadeableSceneObjects.push_back(CONFIRMATION_BUTTON_CRYSTAL_ICON_NAME);
        mScene.AddSceneObject(std::move(crystalIconSo));
        
        // Unlock Bar
        SceneObject unlockBarSo;
        unlockBarSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        unlockBarSo.mSceneObjectType = SceneObjectType::GUIObject;
        unlockBarSo.mPosition = UNLOCK_BAR_POSITION;
        unlockBarSo.mScale = UNLOCK_BAR_SCALE;
        unlockBarSo.mName = UNLOCK_BAR_NAME;
        unlockBarSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mFadeableSceneObjects.push_back(UNLOCK_BAR_NAME);
        mScene.AddSceneObject(std::move(unlockBarSo));
        
        // Unlock Bar Frame
        SceneObject unlockBarFrameSo;
        unlockBarFrameSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        unlockBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
        unlockBarFrameSo.mPosition = UNLOCK_BAR_POSITION;
        unlockBarFrameSo.mScale = UNLOCK_BAR_SCALE;
        unlockBarFrameSo.mName = UNLOCK_BAR_FRAME_NAME;
        unlockBarFrameSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mFadeableSceneObjects.push_back(UNLOCK_BAR_FRAME_NAME);
        mScene.AddSceneObject(std::move(unlockBarFrameSo));
        
        // Unlock Bar Text
        SceneObject unlockBarTextSo;
        unlockBarTextSo.mPosition = UNLOCK_BAR_POSITION + game_constants::BAR_TEXT_OFFSET;
        unlockBarTextSo.mScale = game_constants::BAR_TEXT_SCALE;
        unlockBarTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), game_constants::BAR_TEXT_SCALE, false);
        unlockBarTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        unlockBarTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        unlockBarTextSo.mName = UNLOCK_BAR_TEXT_NAME;
        unlockBarTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        
        auto currentValue = upgradeDefinition.mCrystalUnlockProgress;
        unlockBarTextSo.mText = std::to_string(currentValue) + "/" + std::to_string(upgradeDefinition.mDefaultUnlockCost * GameSingletons::GetResearchCostMultiplier());
        mFadeableSceneObjects.push_back(UNLOCK_BAR_TEXT_NAME);
        mScene.AddSceneObject(std::move(unlockBarTextSo));
    }
    
    // Upgrade Description
    SceneObject upgradeDescriptionSo;
    upgradeDescriptionSo.mPosition = UPGRADE_TEXT_POSITION;
    upgradeDescriptionSo.mScale = UPGRADE_TEXT_SCALE;
    upgradeDescriptionSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    upgradeDescriptionSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
    upgradeDescriptionSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    upgradeDescriptionSo.mName = UPGRADE_TEXT_NAME;
    upgradeDescriptionSo.mText = upgradeDefinition.mUpgradeDescription.GetString();
   
    glm::vec2 botLeftRect, topRightRect;
    scene_object_utils::GetSceneObjectBoundingRect(upgradeDescriptionSo, botLeftRect, topRightRect);
    upgradeDescriptionSo.mPosition.x -= (math::Abs(botLeftRect.x - topRightRect.x)/2.0f);
    upgradeDescriptionSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mFadeableSceneObjects.push_back(UPGRADE_TEXT_NAME);
    mScene.AddSceneObject(std::move(upgradeDescriptionSo));
    
    mVisitedUpgrades.insert(mSelectedUpgrade);
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::OnConfirmationButtonPressed()
{
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    auto confirmationButtonTexFirstLineSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_FIRST_LINE_NAME);
    auto confirmationButtonTexSecondLineSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_SECOND_LINE_NAME);
    auto confirmationButtonCrystalIconSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_CRYSTAL_ICON_NAME);
    
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
        
        confirmationButtonSo.mExtraCompoundingAnimations.clear();
        confirmationButtonSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, ARROW_PULSING_SPEED * 2, ARROW_PULSING_ENLARGEMENT_FACTOR * 10, false));
    }
    
    if (confirmationButtonTexFirstLineSoOpt)
    {
        auto& confirmationButtonTextFirstLineSo = confirmationButtonTexFirstLineSoOpt->get();
        confirmationButtonTextFirstLineSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        
        confirmationButtonTextFirstLineSo.mExtraCompoundingAnimations.clear();
        confirmationButtonTextFirstLineSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonTextFirstLineSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonTextFirstLineSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonTextFirstLineSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_TEXT_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, ARROW_PULSING_SPEED * 2, ARROW_PULSING_ENLARGEMENT_FACTOR / 40, false));
    }
    
    if (confirmationButtonTexSecondLineSoOpt)
    {
        auto& confirmationButtonTextSecondLineSo = confirmationButtonTexSecondLineSoOpt->get();
        confirmationButtonTextSecondLineSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        
        confirmationButtonTextSecondLineSo.mExtraCompoundingAnimations.clear();
        confirmationButtonTextSecondLineSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonTextSecondLineSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonTextSecondLineSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonTextSecondLineSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_TEXT_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, ARROW_PULSING_SPEED * 2, ARROW_PULSING_ENLARGEMENT_FACTOR / 40, false));
    }
    
    if (confirmationButtonCrystalIconSoOpt)
    {
        auto& confirmationButtonCrystalIconSo = confirmationButtonCrystalIconSoOpt->get();
        confirmationButtonCrystalIconSo.mScale = CONFIRMATION_BUTTON_CRYSTAL_ICON_SCALE;
        
        confirmationButtonCrystalIconSo.mExtraCompoundingAnimations.clear();
        confirmationButtonCrystalIconSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonCrystalIconSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonCrystalIconSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonCrystalIconSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_CRYSTAL_ICON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, ARROW_PULSING_SPEED * 2, ARROW_PULSING_ENLARGEMENT_FACTOR / 40, false));
    }
    
    CreateCrystalsTowardTargetPosition(mCurrentOperationCrystalCost, FLYING_CRYSTALS_TARGET_POSITION);
    GameSingletons::SetCrystalCount(GameSingletons::GetCrystalCount() - mCurrentOperationCrystalCost);
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::UpdateFadeableSceneObjects(const float dtMillis)
{
    for (const auto& fadeableSceneObjectName: mFadeableSceneObjects)
    {
        auto sceneObjectOpt = mScene.GetSceneObject(fadeableSceneObjectName);
        if (sceneObjectOpt)
        {
            auto& sceneObject = sceneObjectOpt->get();
            if (mOptionSelectionState == OptionSelectionState::OPTION_NOT_SELECTED)
            {
                sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            else if (fadeableSceneObjectName == CONFIRMATION_BUTTON_CRYSTAL_ICON_NAME || fadeableSceneObjectName == CONFIRMATION_BUTTON_NAME || fadeableSceneObjectName == CONFIRMATION_BUTTON_TEXT_FIRST_LINE_NAME || fadeableSceneObjectName == CONFIRMATION_BUTTON_TEXT_SECOND_LINE_NAME || (mOptionSelectionState == OptionSelectionState::UNLOCK_TEXTURE_TRANSITION && fadeableSceneObjectName != UPGRADE_TEXT_NAME))
            {
                sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
                {
                    sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                }
            }
        }
    }
    
    auto navigationArrowSoOpt = mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME);
    if (navigationArrowSoOpt)
    {
        if (mOptionSelectionState == OptionSelectionState::OPTION_NOT_SELECTED || mOptionSelectionState == OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN)
        {
            navigationArrowSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (navigationArrowSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
            {
                navigationArrowSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            }
        }
        else
        {
            navigationArrowSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (navigationArrowSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
            {
                navigationArrowSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::UpdateUnlockBarSceneObjects(const float dtMillis)
{
    // Player Health Bar update
    auto unlockBarSoOpt = mScene.GetSceneObject(UNLOCK_BAR_NAME);
    auto unlockBarFrameSoOpt = mScene.GetSceneObject(UNLOCK_BAR_FRAME_NAME);
    auto unlockBarTextSoOpt = mScene.GetSceneObject(UNLOCK_BAR_TEXT_NAME);

    if (unlockBarSoOpt && unlockBarFrameSoOpt && unlockBarTextSoOpt)
    {
        auto& unlockBarSo = unlockBarSoOpt->get();
        auto& unlockBarFrameSo = unlockBarFrameSoOpt->get();
        auto& unlockBarTextSo = unlockBarTextSoOpt->get();
        
        unlockBarSo.mPosition = UNLOCK_BAR_POSITION;
        unlockBarSo.mPosition.z = game_constants::PLAYER_HEALTH_BAR_Z;
        
        unlockBarFrameSo.mPosition = UNLOCK_BAR_POSITION;
        
        mSelectedUpgrade = mUpgrades.at(mCarouselController->GetSelectedIndex());
        const auto& upgradeDefinition = GameSingletons::GetAvailableUpgrades().at(mCarouselController->GetSelectedIndex());
        auto currentValue = upgradeDefinition.mCrystalUnlockProgress;
        
        float unlockPerc = currentValue/static_cast<float>(upgradeDefinition.mDefaultUnlockCost * GameSingletons::GetResearchCostMultiplier());
        
        if (unlockPerc > 0.0f)
        {
            unlockBarSo.mInvisible = false;
            unlockBarSo.mScale.x = UNLOCK_BAR_SCALE.x * unlockPerc;
            unlockBarSo.mPosition.x -= (1.0f - unlockPerc)/game_constants::BAR_POSITION_DIVISOR_MAGIC * UNLOCK_BAR_SCALE.x;
        }
        else
        {
            unlockBarSo.mInvisible = true;
        }
        
        unlockBarTextSo.mText = std::to_string(currentValue) + "/" + std::to_string(upgradeDefinition.mDefaultUnlockCost  * GameSingletons::GetResearchCostMultiplier());
        
        glm::vec2 botLeftRect, topRightRect;
        scene_object_utils::GetSceneObjectBoundingRect(unlockBarTextSo, botLeftRect, topRightRect);
        unlockBarTextSo.mPosition = UNLOCK_BAR_POSITION + game_constants::BAR_TEXT_OFFSET;
        unlockBarTextSo.mPosition.x -= (math::Abs(botLeftRect.x - topRightRect.x)/2.0f);
    }
}

///------------------------------------------------------------------------------------------------

void ResearchUpdater::CreateCrystalsTowardTargetPosition(const long crystalCount, const glm::vec3& position)
{
    for (int i = 0; i < crystalCount; ++i)
    {
        mFlows.emplace_back([this, position]()
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
        }, i * game_constants::DROPPED_CRYSTALS_CREATION_STAGGER_MILLIS, RepeatableFlow::RepeatPolicy::ONCE);
    }
}

///------------------------------------------------------------------------------------------------
