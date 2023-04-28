///------------------------------------------------------------------------------------------------
///  ChestRewardUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "CarouselController.h"
#include "ChestRewardUpdater.h"
#include "FullScreenOverlayController.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "LightRepository.h"
#include "ObjectTypeDefinitionRepository.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "datarepos/FontRepository.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

static const char* CHEST_BASE_MESH_FILE_NAME = "chest_base.obj";
static const char* CHEST_LID_MESH_FILE_NAME = "chest_lid.obj";
static const char* CHEST_TEXTURE_FILE_NAME = "reward_chest.bmp";
static const char* CONFIRMATION_BUTTON_TEXTURE_FILE_NAME = "confirmation_button_mm.bmp";
static const char* BOSS_REWARD_TEXT = "BOSS REWARD";

static const strutils::StringId CHEST_BASE_NAME = strutils::StringId("CHEST_BASE");
static const strutils::StringId CHEST_LID_NAME = strutils::StringId("CHEST_LID");
static const strutils::StringId CHEST_LIGHT_NAME = strutils::StringId("CHEST_LIGHT");
static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT");
static const strutils::StringId UPGRADE_TEXT_NAME = strutils::StringId("UPGRADE_TEXT");
static const strutils::StringId REWARD_TITLE_NAME = strutils::StringId("REWARD_SCREEN_TITLE");
static const strutils::StringId OVERLAY_NAME = strutils::StringId("REWARD_OVERLAY");

static const glm::vec3 BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, -7.0f);
static const glm::vec3 CHEST_BASE_POSITION = glm::vec3(0.0f, -1.576f, -4.20f);
static const glm::vec3 CHEST_LID_POSITION = glm::vec3(0.0f, 0.183f, -4.20f);
static const glm::vec3 CHEST_LIGHT_POSITION = glm::vec3(0.0f, -2.5f, -6.0f);
static const glm::vec3 CHEST_SCALE = glm::vec3(2.0f, 2.0f, 2.0f);

static const glm::vec3 CONFIRMATION_BUTTON_POSITION = glm::vec3(0.0f, -6.0f, 0.0f);
static const glm::vec3 CONFIRMATION_BUTTON_SCALE = glm::vec3(3.5f, 3.5f, 0.0f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_POSITION = glm::vec3(-0.8f, -6.3f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_SCALE = glm::vec3(0.007f, 0.007f, 1.0f);
static const glm::vec3 UPGRADE_TEXT_POSITION = glm::vec3(0.25f, 4.0f, 0.5f);
static const glm::vec3 UPGRADE_TEXT_SCALE = glm::vec3(0.01f, 0.01f, 1.0f);

static const glm::vec3 REWARD_SCREEN_TITLE_POSITION = glm::vec3(-4.8f, 7.8f, 2.0f);
static const glm::vec3 REWARD_SCREEN_TITLE_SCALE = glm::vec3(0.014f, 0.014f, 1.0f);
static const glm::vec3 SELECTED_REWARD_VERTICAL_OFFSET = glm::vec3(0.0f, 5.0f, 0.0f);

static const  glm::vec4 CHEST_LIGHT_COLOR = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);

static const float CHEST_LIGHT_INIT_POWER = 10.0f;
static const float CHEST_X_ROTATION = math::PI/6;
static const float CHEST_LIGHT_SPEED = 0.003f;
static const float CHEST_LIGHT_SIN_MULTIPLIER = 1/40.0f;
static const float CHEST_OPENING_ANIMATION_SPEED = 1.0f/800.0f;
static const float CHEST_PULSE_SPEED = 0.005f;
static const float CHEST_SHAKE_RAMP_SPEED = 0.0001f;
static const float CHEST_SHAKE_MAX_MAG = 0.3f;

static const float CONFIRMATION_BUTTON_ROTATION_SPEED = 0.0002f;
static const float CONFIRMATION_BUTTON_PULSING_SPEED = 0.02f;
static const float CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR = 1.0f/10.0f;
static const float CONFIRMATION_BUTTON_TEXT_PULSING_ENLARGEMENT_FACTOR = 1.0f/4000.0f;

static const float SELECTED_REWARD_ROTATION_SPEED = 0.0120f;
static const float SELECTED_REWARD_VERTICAL_SPEED = 0.0008f;
static const float SELECTED_REWARD_SHINE_SPEED = 1.0f/200.0f;

///------------------------------------------------------------------------------------------------

ChestRewardUpdater::ChestRewardUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mUpgradeUnlockedHandler(scene, mBox2dWorld)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mRewardFlowState(RewardFlowState::AWAIT_PRESS)
    , mShakeNoiseMag(0.0f)
    , mChestPulseValueAccum(0.0f)
    , mChestAnimationTweenValue(0.0f)
    , mChestLightDtAccum(0.0f)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif

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
    
    // Chest components
    {
        SceneObject chestBaseSo;
        chestBaseSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CHEST_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + CHEST_BASE_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        
        chestBaseSo.mSceneObjectType = SceneObjectType::GUIObject;
        chestBaseSo.mPosition = CHEST_BASE_POSITION;
        chestBaseSo.mScale = CHEST_SCALE;
        chestBaseSo.mRotation.x = CHEST_X_ROTATION;
        chestBaseSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = true;
        chestBaseSo.mName = CHEST_BASE_NAME;
        mScene.AddSceneObject(std::move(chestBaseSo));
    }
    
    {
        SceneObject chestLidSo;
        chestLidSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CHEST_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + CHEST_LID_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        
        chestLidSo.mSceneObjectType = SceneObjectType::GUIObject;
        chestLidSo.mPosition = CHEST_LID_POSITION;
        chestLidSo.mScale = CHEST_SCALE;
        chestLidSo.mRotation.x = CHEST_X_ROTATION;
        chestLidSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = true;
        chestLidSo.mName = CHEST_LID_NAME;
        mScene.AddSceneObject(std::move(chestLidSo));
    }
    
    mScene.GetLightRepository().AddLight(LightType::POINT_LIGHT, CHEST_LIGHT_NAME, CHEST_LIGHT_COLOR, CHEST_LIGHT_POSITION, CHEST_LIGHT_INIT_POWER);
    mScene.GetLightRepository().AddLight(LightType::AMBIENT_LIGHT, game_constants::AMBIENT_LIGHT_NAME, game_constants::AMBIENT_LIGHT_COLOR, glm::vec3(0.0f), 0.0f);
}

///------------------------------------------------------------------------------------------------

ChestRewardUpdater::~ChestRewardUpdater()
{
    
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective ChestRewardUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE)
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    auto chestBaseSoOpt = mScene.GetSceneObject(CHEST_BASE_NAME);
    auto chestLidSoOpt = mScene.GetSceneObject(CHEST_LID_NAME);
    
    switch (mRewardFlowState)
    {
        case RewardFlowState::AWAIT_PRESS:
        {
            mChestPulseValueAccum += CHEST_PULSE_SPEED * dtMillis;
            
            if (chestBaseSoOpt && chestLidSoOpt)
            {
                auto& chestBaseSo = mScene.GetSceneObject(CHEST_BASE_NAME)->get();
                auto& chestLidSo = mScene.GetSceneObject(CHEST_LID_NAME)->get();
                
                chestBaseSo.mScale.x += math::Sinf(mChestPulseValueAccum) * 1.0f/1000.0f;
                chestBaseSo.mScale.y += math::Sinf(mChestPulseValueAccum) * 1.0f/333.0f;
                chestBaseSo.mScale.z += math::Sinf(mChestPulseValueAccum) * 1.0f/1000.0f;
                
                chestLidSo.mScale.x += math::Sinf(mChestPulseValueAccum) * 1.0f/1000.0f;
                chestLidSo.mScale.y += math::Sinf(mChestPulseValueAccum) * 1.0f/333.0f;
                chestLidSo.mScale.z += math::Sinf(mChestPulseValueAccum) * 1.0f/1000.0f;
                
                auto& inputContext = GameSingletons::GetInputContext();
                if (inputContext.mEventType == SDL_FINGERDOWN)
                {
                    chestBaseSo.mScale = CHEST_SCALE;
                    chestLidSo.mScale = CHEST_SCALE;
                    
                    mRewardFlowState = RewardFlowState::SHAKE;
                }
            }
        } break;
            
        case RewardFlowState::SHAKE:
        {
            if (chestBaseSoOpt && chestLidSoOpt)
            {
                auto& chestBaseSo = mScene.GetSceneObject(CHEST_BASE_NAME)->get();
                auto& chestLidSo = mScene.GetSceneObject(CHEST_LID_NAME)->get();
                
                mShakeNoiseMag += dtMillis * CHEST_SHAKE_RAMP_SPEED;
                mShakeNoiseMag = math::Min(CHEST_SHAKE_MAX_MAG, mShakeNoiseMag);
                
                auto randomOffset = glm::vec3(math::RandomFloat(-mShakeNoiseMag, mShakeNoiseMag), math::RandomFloat(-mShakeNoiseMag, mShakeNoiseMag), 0.0f);
                
                chestBaseSo.mPosition = CHEST_BASE_POSITION + randomOffset;
                chestLidSo.mPosition = CHEST_LID_POSITION + randomOffset;
                
                if (mShakeNoiseMag >= CHEST_SHAKE_MAX_MAG)
                {
                    chestBaseSo.mPosition -= randomOffset;
                    chestLidSo.mPosition -= randomOffset;
                    
                    mRewardFlowState = RewardFlowState::OPEN_ANIMATION;
                }
            }
        } break;
            
        case RewardFlowState::OPEN_ANIMATION:
        {
            mChestLightDtAccum += CHEST_LIGHT_SPEED * dtMillis;
            mChestAnimationTweenValue += CHEST_OPENING_ANIMATION_SPEED * dtMillis;
            
            if (chestBaseSoOpt && chestLidSoOpt)
            {
                auto& chestLidSo = mScene.GetSceneObject(CHEST_LID_NAME)->get();
                
                mChestAnimationTweenValue = math::Min(1.0f, mChestAnimationTweenValue + dtMillis * CHEST_OPENING_ANIMATION_SPEED);
                float perc = math::Min(1.0f, math::TweenValue(mChestAnimationTweenValue, math::BounceFunction, math::TweeningMode::EASE_IN));
                
                chestLidSo.mRotation.x = CHEST_X_ROTATION - 1.5f * perc;
                
                auto chestLightIndex = mScene.GetLightRepository().GetLightIndex(CHEST_LIGHT_NAME);
                mScene.GetLightRepository().SetLightPower(chestLightIndex, mScene.GetLightRepository().GetLightPower(chestLightIndex) + math::Sinf(mChestLightDtAccum) * CHEST_LIGHT_SIN_MULTIPLIER);
                
                if (perc >= 1.0f && !mScreenOverlayController)
                {
                    mScreenOverlayController = std::make_unique<FullScreenOverlayController>(mScene, game_constants::FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED, game_constants::FULL_SCREEN_OVERLAY_MENU_MAX_ALPHA, true, [&]()
                    {
                        CreateRewardObjects();
                        mRewardFlowState = RewardFlowState::REWARD_SELECTION;
                    }, nullptr, -1.0f, OVERLAY_NAME, false);
                }
            }
        } break;
            
        case RewardFlowState::REWARD_SELECTION:
        {
            // Reward screen title
            auto rewardScreenTitleSoOpt = mScene.GetSceneObject(REWARD_TITLE_NAME);
            if (rewardScreenTitleSoOpt)
            {
                rewardScreenTitleSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Min(1.0f, rewardScreenTitleSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] + game_constants::TEXT_FADE_IN_ALPHA_SPEED * dtMillis);
            }
            
            // Fade in confirmation button
            auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
            if (confirmationButtonSoOpt)
            {
                auto& confirmationButtonSo = confirmationButtonSoOpt->get();
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            
            // & text
            auto confirmationButtonTextSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
            if (confirmationButtonTextSoOpt)
            {
                auto& confirmationButtonTextSo = confirmationButtonTextSoOpt->get();
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            
            // & upgrade text
            auto upgradeTextSoOpt = mScene.GetSceneObject(UPGRADE_TEXT_NAME);
            if (upgradeTextSoOpt)
            {
                auto& upgradeTextSo = upgradeTextSoOpt->get();
                upgradeTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (upgradeTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    upgradeTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
            
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
                    mRewardFlowState = RewardFlowState::REWARD_SELECTED_ANIMATION_HIGH;
                }
            }
            
            mCarouselController->Update(dtMillis);
        } break;
            
        case RewardFlowState::REWARD_SELECTED_ANIMATION_HIGH:
        {
            auto& selectedRewardSo = mCarouselController->GetSelectedSceneObject()->get();
            selectedRewardSo.mPosition.z = 2.0f;
            
            // Fade out confirmation button
            auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
            if (confirmationButtonSoOpt)
            {
                auto& confirmationButtonSo = confirmationButtonSoOpt->get();
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
                {
                    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_NAME);
                    confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                }
            }
            
            // & text
            auto confirmationButtonTextSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
            if (confirmationButtonTextSoOpt)
            {
                auto& confirmationButtonTextSo = confirmationButtonTextSoOpt->get();
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
                {
                    confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_TEXT_NAME);
                }
            }
        } break;
            
        case RewardFlowState::CREATE_REWARD_SELECTED_USAGE_ANIMATION:
        {
            auto upgradeDef = FindSelectedRewardDefinition();
            mUpgradeUnlockedHandler.OnUpgradeGained(upgradeDef.mUpgradeNameId);
            mRewardFlowState = RewardFlowState::REWARD_SELECTED_USAGE_ANIMATING;
        } break;
            
        case RewardFlowState::REWARD_SELECTED_USAGE_ANIMATING:
        {
            if (mUpgradeUnlockedHandler.Update(dtMillis) == UpgradeUnlockedHandler::UpgradeAnimationState::FINISHED)
            {
                GameSingletons::SetMapLevel(GameSingletons::GetMapLevel() + 1);
                GameSingletons::SetMapGenerationSeed(math::RandomInt());
                GameSingletons::SetBackgroundIndex(GameSingletons::GetMapGenerationSeed() % game_constants::BACKGROUND_COUNT);
                GameSingletons::SetCurrentMapCoord(MapCoord(game_constants::DEFAULT_MAP_COORD_COL, game_constants::DEFAULT_MAP_COORD_ROW));
                mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
                mRewardFlowState = RewardFlowState::TRANSITIONING;
            }
        } break;
            
        case RewardFlowState::TRANSITIONING:
        {
        } break;
    }
    
    // Animate overlay controller
    if (mScreenOverlayController)
    {
        mScreenOverlayController->Update(dtMillis);
    }
    
    // Animate all SOs
    for (auto& sceneObject: sceneObjects)
    {
        // Check if this scene object has a respective family object definition
        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject.mObjectFamilyTypeName);
        if (sceneObjectTypeDefOpt && !sceneObject.mCustomDrivenMovement)
        {
            // Update movement
            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
            switch (sceneObjectTypeDef.mMovementControllerPattern)
            {
                case MovementControllerPattern::CONSTANT_VELOCITY:
                {
                    sceneObject.mBody->SetLinearVelocity(b2Vec2(sceneObjectTypeDef.mConstantLinearVelocity.x, sceneObjectTypeDef.mConstantLinearVelocity.y));
                } break;
                default: break;
            }
        }
        
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

void ChestRewardUpdater::VOnAppStateChange(Uint32 event)
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

std::string ChestRewardUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId ChestRewardUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void ChestRewardUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

UpgradeDefinition ChestRewardUpdater::FindSelectedRewardDefinition() const
{
    int counter = 0;
    for (const auto& availableUpgrade: GameSingletons::GetAvailableUpgrades())
    {
        if (availableUpgrade.mUnlockCost == 0)
        {
            if (counter++ == mCarouselController->GetSelectedIndex())
            {
                return availableUpgrade;
            }
        }
    }
    
    return GameSingletons::GetAvailableUpgrades()[mCarouselController->GetSelectedIndex()];
}

///------------------------------------------------------------------------------------------------

void ChestRewardUpdater::CreateRewardObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    SceneObject rewardScreenTitleSo;
    rewardScreenTitleSo.mScale = REWARD_SCREEN_TITLE_SCALE;
    rewardScreenTitleSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    rewardScreenTitleSo.mFontName = game_constants::DEFAULT_FONT_NAME;
    rewardScreenTitleSo.mSceneObjectType = SceneObjectType::GUIObject;
    rewardScreenTitleSo.mName = REWARD_TITLE_NAME;
    rewardScreenTitleSo.mPosition = REWARD_SCREEN_TITLE_POSITION;
    rewardScreenTitleSo.mText = BOSS_REWARD_TEXT;
    
    rewardScreenTitleSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    mScene.AddSceneObject(std::move(rewardScreenTitleSo));
    
    std::vector<resources::ResourceId> upgradeTextureIds;
    for (const auto& upgradeEntry: GameSingletons::GetAvailableUpgrades())
    {
        if (upgradeEntry.mUnlockCost == 0)
        {
            upgradeTextureIds.push_back(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + upgradeEntry.mTextureFileName));
        }
    }
    
    mCarouselController = std::make_unique<CarouselController>(mScene, upgradeTextureIds, [&](){ OnCarouselMovementStart(); }, [&](){ OnCarouselStationary(); }, 0.0f);
}

///------------------------------------------------------------------------------------------------

void ChestRewardUpdater::OnConfirmationButtonPressed()
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
    
    mScene.RemoveAllSceneObjectsWithName(UPGRADE_TEXT_NAME);
    
    auto& selectedRewardSo = mCarouselController->GetSelectedSceneObject()->get();
    
    selectedRewardSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(selectedRewardSo.mAnimation->VGetCurrentTextureResourceId(), selectedRewardSo.mAnimation->VGetCurrentMeshResourceId(), selectedRewardSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, SELECTED_REWARD_ROTATION_SPEED, false));
    
    glm::vec3 startPos = selectedRewardSo.mPosition;
    glm::vec3 endPos = startPos + SELECTED_REWARD_VERTICAL_OFFSET;
    math::BezierCurve curve({startPos, endPos});
    
    selectedRewardSo.mExtraCompoundingAnimations.push_back(std::make_unique<BezierCurvePathAnimation>(selectedRewardSo.mAnimation->VGetCurrentTextureResourceId(), selectedRewardSo.mAnimation->VGetCurrentMeshResourceId(), selectedRewardSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.0f), curve, SELECTED_REWARD_VERTICAL_SPEED, false));
    selectedRewardSo.mExtraCompoundingAnimations.back()->SetCompletionCallback([&]()
    {
        auto& rewardSo = mCarouselController->GetSelectedSceneObject()->get();
        rewardSo.mRotation.y = 0.0f;
        rewardSo.mExtraCompoundingAnimations.clear();
        rewardSo.mAnimation = std::make_unique<ShineAnimation>(&rewardSo, rewardSo.mAnimation->VGetCurrentTextureResourceId(), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::UPGRADE_SHINE_EFFECT_TEXTURE_FILE_NAME), rewardSo.mAnimation->VGetCurrentMeshResourceId(), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::SHINE_SHADER_FILE_NAME), glm::vec3(1.0f), SELECTED_REWARD_SHINE_SPEED, false);
        rewardSo.mAnimation->SetCompletionCallback([&]()
        {
            mRewardFlowState = RewardFlowState::CREATE_REWARD_SELECTED_USAGE_ANIMATION;
        });
    });
}

///------------------------------------------------------------------------------------------------

void ChestRewardUpdater::OnCarouselStationary()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
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
    
    // Upgrade Description
    SceneObject upgradeDescriptionSo;
    upgradeDescriptionSo.mPosition = UPGRADE_TEXT_POSITION;
    upgradeDescriptionSo.mScale = UPGRADE_TEXT_SCALE;
    upgradeDescriptionSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    upgradeDescriptionSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
    upgradeDescriptionSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    upgradeDescriptionSo.mName = UPGRADE_TEXT_NAME;
    upgradeDescriptionSo.mText = FindSelectedRewardDefinition().mUpgradeDescription.GetString();
   
    glm::vec2 botLeftRect, topRightRect;
    scene_object_utils::GetSceneObjectBoundingRect(upgradeDescriptionSo, botLeftRect, topRightRect);
    upgradeDescriptionSo.mPosition.x -= (math::Abs(botLeftRect.x - topRightRect.x)/2.0f);
    upgradeDescriptionSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene.AddSceneObject(std::move(upgradeDescriptionSo));
}

///------------------------------------------------------------------------------------------------

void ChestRewardUpdater::OnCarouselMovementStart()
{
    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_NAME);
    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_TEXT_NAME);
    mScene.RemoveAllSceneObjectsWithName(UPGRADE_TEXT_NAME);
}

///------------------------------------------------------------------------------------------------
