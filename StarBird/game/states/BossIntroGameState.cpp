///------------------------------------------------------------------------------------------------
///  BossIntroGameState.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/03/2023
///------------------------------------------------------------------------------------------------

#include "BossIntroGameState.h"
#include "FightingWaveGameState.h"
#include "SceneObjectUtils.h"
#include "../Scene.h"
#include "../LevelUpdater.h"
#include "../GameConstants.h"
#include "../GameSingletons.h"
#include "../datarepos/FontRepository.h"
#include "../datarepos/ObjectTypeDefinitionRepository.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId BossIntroGameState::STATE_NAME("BossIntroGameState");

static const glm::vec3 BOSS_INTRO_TEXT_INIT_POS = glm::vec3(0.5f, 0.0f, 2.0f);
static const glm::vec3 BOSS_INTRO_TEXT_SCALE = glm::vec3(0.02f, 0.02f, 1.0f);

///------------------------------------------------------------------------------------------------

void BossIntroGameState::VInitialize()
{
    mSubState = SubState::BOSS_NAME_DISPLAY;
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Boss Text
    SceneObject bossNameTextSO;
    bossNameTextSO.mPosition = BOSS_INTRO_TEXT_INIT_POS;
    bossNameTextSO.mScale = BOSS_INTRO_TEXT_SCALE;
    bossNameTextSO.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    bossNameTextSO.mFontName = game_constants::DEFAULT_FONT_NAME;
    bossNameTextSO.mSceneObjectType = SceneObjectType::GUIObject;
    bossNameTextSO.mName = game_constants::BOSS_INTRO_TEXT_SCENE_OBJECT_NAME;
    bossNameTextSO.mText = mLevelUpdater->GetCurrentLevelDefinition().mWaves[mLevelUpdater->GetCurrentWaveNumber()].mBossName.GetString();
    
    glm::vec2 botLeftRect, topRightRect;
    scene_object_utils::GetSceneObjectBoundingRect(bossNameTextSO, botLeftRect, topRightRect);
    bossNameTextSO.mPosition = BOSS_INTRO_TEXT_INIT_POS;
    bossNameTextSO.mPosition.x -= (math::Abs(botLeftRect.x - topRightRect.x)/2.0f);
    
    bossNameTextSO.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    mScene->AddSceneObject(std::move(bossNameTextSO));
    
    // Boss Health Bar
    SceneObject bossHealthBarSo;
    bossHealthBarSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BOSS_HEALTH_BAR_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    bossHealthBarSo.mSceneObjectType = SceneObjectType::GUIObject;
    bossHealthBarSo.mPosition = game_constants::BOSS_HEALTH_BAR_POSITION;
    bossHealthBarSo.mPosition.z = game_constants::BOSS_HEALTH_BAR_Z;
    bossHealthBarSo.mScale = game_constants::BOSS_HEALTH_BAR_SCALE;
    bossHealthBarSo.mName = game_constants::BOSS_HEALTH_BAR_SCENE_OBJECT_NAME;
    bossHealthBarSo.mInvisible = true;
    mScene->AddSceneObject(std::move(bossHealthBarSo));
    
    // Boss Health Bar Frame
    SceneObject bossHealthBarFrameSo;
    bossHealthBarFrameSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BOSS_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    bossHealthBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
    bossHealthBarFrameSo.mPosition = game_constants::BOSS_HEALTH_BAR_POSITION;
    bossHealthBarFrameSo.mScale = game_constants::BOSS_HEALTH_BAR_SCALE;
    bossHealthBarFrameSo.mName = game_constants::BOSS_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME;
    bossHealthBarFrameSo.mInvisible = true;
    mScene->AddSceneObject(std::move(bossHealthBarFrameSo));
    
    // Player Health Bar Text
    SceneObject healthBarTextSo;
    healthBarTextSo.mPosition = game_constants::BOSS_HEALTH_BAR_POSITION + game_constants::HEALTH_BAR_TEXT_OFFSET;
    healthBarTextSo.mScale = game_constants::HEALTH_BAR_TEXT_SCALE;
    healthBarTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), game_constants::HEALTH_BAR_TEXT_SCALE, false);
    healthBarTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
    healthBarTextSo.mSceneObjectType = SceneObjectType::GUIObject;
    healthBarTextSo.mName = game_constants::BOSS_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME;
    healthBarTextSo.mText = std::to_string(static_cast<int>(mLevelUpdater->GetCurrentLevelDefinition().mWaves.at(mLevelUpdater->GetCurrentWaveNumber()).mBossHealth));
    healthBarTextSo.mInvisible = true;
    
    mScene->AddSceneObject(std::move(healthBarTextSo));
    
    mLevelUpdater->AddFlow(RepeatableFlow([&]()
    {
        mScene->RemoveAllSceneObjectsWithName(game_constants::BOSS_INTRO_TEXT_SCENE_OBJECT_NAME);
        mSubState = SubState::BOSS_HEALTH_BAR_ANIMATION;
        GameSingletons::SetBossCurrentHealth(1.0f);
        GameSingletons::SetBossMaxHealth(mLevelUpdater->GetCurrentLevelDefinition().mWaves.at(mLevelUpdater->GetCurrentWaveNumber()).mBossHealth);
    }, game_constants::BOSS_INTRO_DURATION_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_constants::BOSS_INTRO_FLOW_NAME));
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective BossIntroGameState::VUpdate(const float dtMillis)
{
    switch (mSubState)
    {
        case SubState::BOSS_NAME_DISPLAY:
        {
            auto bossNameTextSoOpt = mScene->GetSceneObject(game_constants::BOSS_INTRO_TEXT_SCENE_OBJECT_NAME);
            auto bossIntroFlowOpt = mLevelUpdater->GetFlow(game_constants::BOSS_INTRO_FLOW_NAME);
            
            if (bossNameTextSoOpt && bossIntroFlowOpt)
            {
                auto& bossNameTextIntroSo = bossNameTextSoOpt->get();
                auto& bossIntroFlow = bossIntroFlowOpt->get();
                
                const auto ticksLeft = bossIntroFlow.GetTicksLeft();
                const auto halfDuration = bossIntroFlow.GetDuration()/2;
                if (ticksLeft > halfDuration)
                {
                    bossNameTextIntroSo.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].w = 1.0f - (ticksLeft - halfDuration)/halfDuration;
                }
                else
                {
                    bossNameTextIntroSo.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].w = ticksLeft/halfDuration;
                }
            }
        } break;
            
        case SubState::BOSS_HEALTH_BAR_ANIMATION:
        {
            mScene->GetSceneObject(game_constants::BOSS_HEALTH_BAR_SCENE_OBJECT_NAME)->get().mInvisible = false;
            mScene->GetSceneObject(game_constants::BOSS_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME)->get().mInvisible = false;
            mScene->GetSceneObject(game_constants::BOSS_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME)->get().mInvisible = false;
            
            GameSingletons::SetBossCurrentHealth(GameSingletons::GetBossCurrentHealth() + (GameSingletons::GetBossMaxHealth()/100.0f) * game_constants::BOSS_INTRO_ANIMATED_HEALTH_SPEED * dtMillis);
            if (GameSingletons::GetBossCurrentHealth() >= GameSingletons::GetBossMaxHealth())
            {
                GameSingletons::SetBossCurrentHealth(GameSingletons::GetBossMaxHealth());
                Complete();
            }
        } break;
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void BossIntroGameState::VDestroy()
{
    
}

///------------------------------------------------------------------------------------------------
