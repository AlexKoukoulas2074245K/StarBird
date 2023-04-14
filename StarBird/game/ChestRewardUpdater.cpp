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

static const strutils::StringId CHEST_BASE_NAME = strutils::StringId("CHEST_BASE");
static const strutils::StringId CHEST_LID_NAME = strutils::StringId("CHEST_LID");
static const strutils::StringId CHEST_LIGHT_NAME = strutils::StringId("CHEST_LIGHT");

static const glm::vec3 BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, -7.0f);
static const glm::vec3 CHEST_BASE_POSITION = glm::vec3(0.0f, -1.576f, -4.20f);
static const glm::vec3 CHEST_LID_POSITION = glm::vec3(0.0f, 0.183f, -4.20f);
static const glm::vec3 CHEST_LIGHT_POSITION = glm::vec3(0.0f, -2.5f, -6.0f);
static const glm::vec3 CHEST_SCALE = glm::vec3(2.0f, 2.0f, 2.0f);

static const  glm::vec4 CHEST_LIGHT_COLOR = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);

static const float CHEST_LIGHT_INIT_POWER = 10.0f;
static const float CHEST_X_ROTATION = math::PI/6;
static const float CHEST_LIGHT_SPEED = 0.003f;
static const float CHEST_LIGHT_SIN_MULTIPLIER = 1/40.0f;
static const float CHEST_OPENING_ANIMATION_SPEED = 1.0f/800.0f;

///------------------------------------------------------------------------------------------------

ChestRewardUpdater::ChestRewardUpdater(Scene& scene)
    : mScene(scene)
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
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
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
            mChestPulseValueAccum += 0.005f * dtMillis;
            
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
                
                mShakeNoiseMag += dtMillis * 0.0001f;
                mShakeNoiseMag = math::Min(0.3f, mShakeNoiseMag);
                
                auto randomOffset = glm::vec3(math::RandomFloat(-mShakeNoiseMag, mShakeNoiseMag), math::RandomFloat(-mShakeNoiseMag, mShakeNoiseMag), 0.0f);
                
                chestBaseSo.mPosition = CHEST_BASE_POSITION + randomOffset;
                chestLidSo.mPosition = CHEST_LID_POSITION + randomOffset;
                
                if (mShakeNoiseMag >= 0.3f)
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
                        auto& resService = resources::ResourceLoadingService::GetInstance();
                        
                        SceneObject rewardScreenTitleSo;
                        rewardScreenTitleSo.mScale = glm::vec3(0.014f, 0.014f, 1.0f);
                        rewardScreenTitleSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
                        rewardScreenTitleSo.mFontName = game_constants::DEFAULT_FONT_NAME;
                        rewardScreenTitleSo.mSceneObjectType = SceneObjectType::GUIObject;
                        rewardScreenTitleSo.mName = strutils::StringId("REWARD_SCREEN_TITLE");
                        rewardScreenTitleSo.mPosition = glm::vec3(-4.8f, 7.8f, 5.0f);
                        rewardScreenTitleSo.mText = "BOSS REWARD";
                        
                        rewardScreenTitleSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                        
                        mScene.AddSceneObject(std::move(rewardScreenTitleSo));
                        
                        std::vector<resources::ResourceId> upgradeTextureIds;
                        for (const auto& availableUpgradeMapEntry: GameSingletons::GetAvailableUpgrades())
                        {
                            upgradeTextureIds.push_back(availableUpgradeMapEntry.second.mTextureResourceId);
                        }
                        
                        mCarouselController = std::make_unique<CarouselController>(mScene, upgradeTextureIds, nullptr, nullptr, 0.0f);
                        
                        mRewardFlowState = RewardFlowState::REWARD_SELECTION;
                    }, nullptr, -1.0f);
                }
            }
        } break;
            
        case RewardFlowState::REWARD_SELECTION:
        {
            auto rewardScreenTitleSoOpt = mScene.GetSceneObject(strutils::StringId("REWARD_SCREEN_TITLE"));
            if (rewardScreenTitleSoOpt)
            {
                rewardScreenTitleSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Min(1.0f, rewardScreenTitleSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] + game_constants::TEXT_FADE_IN_ALPHA_SPEED * dtMillis);
            }
            
            mCarouselController->Update(dtMillis);
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

