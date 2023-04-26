///------------------------------------------------------------------------------------------------
///  LabUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "CarouselController.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "LabUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "TextPromptController.h"
#include "states/DebugConsoleGameState.h"
#include "datarepos/FontRepository.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

#include <vector>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

static const std::vector<game_constants::LabOptionType> DEFAULT_LAB_OPTIONS =
{
    game_constants::LabOptionType::REPAIR,
    game_constants::LabOptionType::STATS_UPGRADE,
    game_constants::LabOptionType::RESEARCH,
};

static const std::unordered_map<game_constants::LabOptionType, std::string> LAB_OPTION_DESCRIPTIONS =
{
    { game_constants::LabOptionType::REPAIR, "REPAIR:\n Fully repairs the vessel to factory state standards." },
    { game_constants::LabOptionType::STATS_UPGRADE, "STATS UPGRADE:\n Uses a small amount of crystals to upgrade individual vessel parts." },
    { game_constants::LabOptionType::RESEARCH, "RESEARCH:\n Consumes the maximum amount of crystals possible to unlock powerful end-of-map upgrades for the vessel." }
};

static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT");

static const char* CONFIRMATION_BUTTON_TEXTURE_FILE_NAME = "confirmation_button_mm.bmp";
static const char* TEXT_PROMPT_TEXTURE_FILE_NAME = "text_prompt_mm.bmp";

static const glm::vec3 LAB_BACKGROUND_POS = glm::vec3(-1.8f, 0.0f, -1.0f);
static const glm::vec3 LAB_BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);

static const glm::vec3 CONFIRMATION_BUTTON_POSITION = glm::vec3(0.0f, -8.0f, 0.0f);
static const glm::vec3 CONFIRMATION_BUTTON_SCALE = glm::vec3(3.5f, 3.5f, 0.0f);

static const glm::vec3 LAB_REJECTION_TEXT_POSITION = glm::vec3(-3.5f, -6.4f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_POSITION = glm::vec3(-0.8f, -8.3f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_SCALE = glm::vec3(0.007f, 0.007f, 1.0f);

static const glm::vec3 TEXT_PROMPT_POSITION = glm::vec3(0.0f, 7.2f, 0.5f);
static const glm::vec3 TEXT_PROMPT_SCALE = glm::vec3(12.0f, 10.0f, 1.0f);

static const float CONFIRMATION_BUTTON_ROTATION_SPEED = 0.0002f;

static const float CONFIRMATION_BUTTON_PULSING_SPEED = 0.02f;
static const float CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR = 1.0f/10.0f;
static const float CONFIRMATION_BUTTON_TEXT_PULSING_ENLARGEMENT_FACTOR = 1.0f/4000.0f;

///------------------------------------------------------------------------------------------------

LabUpdater::LabUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mUpgradeUnlockedHandler(scene, box2dWorld)
    , mOptionSelectionState(OptionSelectionState::OPTION_NOT_SELECTED)
    , mSelectedLabOption(game_constants::LabOptionType::REPAIR)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    CreateSceneObjects();
    OnCarouselStationary();
}

///------------------------------------------------------------------------------------------------

LabUpdater::~LabUpdater()
{
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective LabUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
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
                    mOptionSelectionState = OptionSelectionState::OPTION_SELECTED;
                }
            }
        
            // Update carousel
            mCarouselController->Update(dtMillis);
            
        } break;
            
        case OptionSelectionState::OPTION_SELECTED:
        {
            auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
            if (confirmationButtonSoOpt && !confirmationButtonSoOpt->get().mExtraCompoundingAnimations.empty() && confirmationButtonSoOpt->get().mExtraCompoundingAnimations.front()->VIsPaused())
            {
                OnTriggerOptionFlow();
                mOptionSelectionState = OptionSelectionState::OPTION_TRIGGERED;
            }
        } break;
            
        case OptionSelectionState::OPTION_TRIGGERED:
        {
            switch (mSelectedLabOption)
            {
                case game_constants::LabOptionType::REPAIR:
                {
                    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_NAME);
                    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_TEXT_NAME);
                    
                    if (GameSingletons::GetPlayerDisplayedHealth() >= GameSingletons::GetPlayerMaxHealth())
                    {
                        mOptionSelectionState = OptionSelectionState::OPTION_FLOW_FINISHED;
                    }
                } break;
                case game_constants::LabOptionType::STATS_UPGRADE:
                {
                    mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::STATS_UPGRADE, "", true));
                    mOptionSelectionState = OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN;
                } break;
                    
                case game_constants::LabOptionType::RESEARCH:
                {
                    mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::RESEARCH, "", true));
                    mOptionSelectionState = OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN;
                } break;
            }
        } break;
            
        case OptionSelectionState::OPTION_FLOW_FINISHED:
        {
            if (mUpgradeUnlockedHandler.Update(dtMillis) == UpgradeUnlockedHandler::UpgradeAnimationState::FINISHED)
            {
                mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
                mOptionSelectionState = OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN;
            }
        } break;
            
        case OptionSelectionState::TRANSITIONING_TO_NEXT_SCREEN:
        {
            return PostStateUpdateDirective::BLOCK_UPDATE;
        } break;
    }
    
    // Fade in confirmation button
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        
        if (mOptionSelectionState == OptionSelectionState::OPTION_NOT_SELECTED)
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
        
        if (mOptionSelectionState == OptionSelectionState::OPTION_NOT_SELECTED)
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
    
    // & text area
    if (mTextPromptController)
    {
        mTextPromptController->Update(dtMillis);
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

void LabUpdater::VOnAppStateChange(Uint32 event)
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

std::string LabUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId LabUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void LabUpdater::VOpenDebugConsole()
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
        bgSO.mScale = LAB_BACKGROUND_SCALE;
        bgSO.mPosition = LAB_BACKGROUND_POS;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }

    // Options
    const auto optionCount = DEFAULT_LAB_OPTIONS.size();
    mLabOptions.resize(optionCount);
    
    std::vector<resources::ResourceId> labOptionTextures;
    
    for (int i = 0; i < static_cast<int>(mLabOptions.size()); ++i)
    {
        game_constants::LabOptionType currentLabOption = static_cast<game_constants::LabOptionType>(i);
        mLabOptions[i] = currentLabOption;
        labOptionTextures.push_back(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_OPTION_TYPE_TO_TEXTURE.at(currentLabOption)));
    }
    
    mCarouselController = std::make_unique<CarouselController>(mScene, labOptionTextures, [&](){ OnCarouselMovementStart(); }, [&](){ OnCarouselStationary(); });
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnCarouselMovementStart()
{
    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_NAME);
    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_TEXT_NAME);
    mTextPromptController.reset();
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnCarouselStationary()
{
    mSelectedLabOption = static_cast<game_constants::LabOptionType>(mCarouselController->GetSelectedIndex());
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    auto validityRejectionText = CheckForOptionValidity();
    if (!validityRejectionText.empty())
    {
        // Confirmation button text
        SceneObject rejectionTextSo;
        rejectionTextSo.mPosition = LAB_REJECTION_TEXT_POSITION;
        rejectionTextSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        rejectionTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        rejectionTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        rejectionTextSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        rejectionTextSo.mName = CONFIRMATION_BUTTON_TEXT_NAME;
        rejectionTextSo.mText = validityRejectionText;
        rejectionTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mScene.AddSceneObject(std::move(rejectionTextSo));
    }
    else
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
        mScene.AddSceneObject(std::move(confirmationButtonSo));
        
        // Confirmation button text
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
    
    // Text Prompt
    mTextPromptController = std::make_unique<TextPromptController>(mScene, std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + TEXT_PROMPT_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false), TEXT_PROMPT_POSITION, TEXT_PROMPT_SCALE, !mVisitedLabOptions.contains(mSelectedLabOption), LAB_OPTION_DESCRIPTIONS.at(mSelectedLabOption));
    
    
    mVisitedLabOptions.insert(mSelectedLabOption);
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnConfirmationButtonPressed()
{
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    auto confirmationButtonTexSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
    
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
        
        confirmationButtonSo.mExtraCompoundingAnimations.clear();
        confirmationButtonSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONFIRMATION_BUTTON_PULSING_SPEED, CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR, false));
    }
    
    if (confirmationButtonTexSoOpt)
    {
        auto& confirmationButtonTextSo = confirmationButtonTexSoOpt->get();
        confirmationButtonTextSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        
        confirmationButtonTextSo.mExtraCompoundingAnimations.clear();
        confirmationButtonTextSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonTextSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_TEXT_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONFIRMATION_BUTTON_PULSING_SPEED, CONFIRMATION_BUTTON_TEXT_PULSING_ENLARGEMENT_FACTOR, false));
    }
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnTriggerOptionFlow()
{
    switch (mSelectedLabOption)
    {
        case game_constants::LabOptionType::REPAIR:
        {
            mUpgradeUnlockedHandler.OnUpgradeGained(game_constants::PLAYER_HEALTH_POTION_UGPRADE_NAME);
        } break;
            
        case game_constants::LabOptionType::STATS_UPGRADE:
        {
            
        } break;
            
        case game_constants::LabOptionType::RESEARCH:
        {
            
        } break;
    }
}

///------------------------------------------------------------------------------------------------

std::string LabUpdater::CheckForOptionValidity() const
{
    if (mSelectedLabOption == game_constants::LabOptionType::REPAIR && GameSingletons::GetPlayerCurrentHealth() == GameSingletons::GetPlayerMaxHealth())
    {
        return "Vessel is not damaged";
    }

    return "";
}

///------------------------------------------------------------------------------------------------
