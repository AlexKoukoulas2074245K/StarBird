///------------------------------------------------------------------------------------------------
///  WaveIntroGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "WaveIntroGameState.h"
#include "BossIntroGameState.h"
#include "ClearedLevelAnimationGameState.h"
#include "FightingWaveGameState.h"
#include "../Scene.h"
#include "../LevelUpdater.h"
#include "../GameConstants.h"
#include "../datarepos/FontRepository.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId WaveIntroGameState::STATE_NAME("WaveIntroGameState");

static const float WAVE_INTRO_DURATION_MILLIS = 3000.0f;

static const glm::vec3 WAVE_INTRO_TEXT_INIT_POS = glm::vec3(-3.0f, 0.0f, 2.0f);
static const glm::vec3 CLEARED_TEXT_INIT_POS = glm::vec3(-3.93, 0.0f, 2.0f);
static const glm::vec3 WAVE_INTRO_TEXT_SCALE = glm::vec3(0.02f, 0.02f, 1.0f);

///------------------------------------------------------------------------------------------------

void WaveIntroGameState::VInitialize()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    SceneObject waveTextSO;
    waveTextSO.mScale = WAVE_INTRO_TEXT_SCALE;
    waveTextSO.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    waveTextSO.mFontName = game_constants::DEFAULT_FONT_NAME;
    waveTextSO.mSceneObjectType = SceneObjectType::GUIObject;
    waveTextSO.mName = game_constants::WAVE_INTRO_TEXT_SCENE_OBJECT_NAME;
    
    if (mLevelUpdater->LevelFinished())
    {
        waveTextSO.mPosition = CLEARED_TEXT_INIT_POS;
        waveTextSO.mText = "CLEARED";
    }
    else
    {
        waveTextSO.mPosition = WAVE_INTRO_TEXT_INIT_POS;
        waveTextSO.mText = "WAVE " + std::to_string(mLevelUpdater->GetCurrentWaveNumber() + 1);
    }
    
    waveTextSO.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene->AddSceneObject(std::move(waveTextSO));
    
    mLevelUpdater->AddFlow(RepeatableFlow([&]()
    {
        if (mLevelUpdater->LevelFinished())
        {
            mLevelUpdater->GetFlow(game_constants::PLAYER_BULLET_FLOW_NAME)->get().ForceFinish();
            Complete(ClearedLevelAnimationGameState::STATE_NAME);
        }
        else
        {
            Complete(FightingWaveGameState::STATE_NAME);
        }
        
    }, WAVE_INTRO_DURATION_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_constants::WAVE_INTRO_FLOW_NAME));
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective WaveIntroGameState::VUpdate(const float dtMillis)
{
    auto waveTextIntroSoOpt = mScene->GetSceneObject(game_constants::WAVE_INTRO_TEXT_SCENE_OBJECT_NAME);
    auto waveTextIntroFlowOpt = mLevelUpdater->GetFlow(game_constants::WAVE_INTRO_FLOW_NAME);
    
    if (waveTextIntroSoOpt && waveTextIntroFlowOpt)
    {
        auto& waveTextIntroSo = waveTextIntroSoOpt->get();
        auto& waveTextIntroFlow = waveTextIntroFlowOpt->get();
        
        const auto ticksLeft = waveTextIntroFlow.GetTicksLeft();
        const auto halfDuration = waveTextIntroFlow.GetDuration()/2;
        if (ticksLeft > halfDuration)
        {
            waveTextIntroSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f - (ticksLeft - halfDuration)/halfDuration;
        }
        else
        {
            waveTextIntroSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = ticksLeft/halfDuration;
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void WaveIntroGameState::VDestroy()
{
    mScene->RemoveAllSceneObjectsWithName(game_constants::WAVE_INTRO_TEXT_SCENE_OBJECT_NAME);
}

///------------------------------------------------------------------------------------------------
