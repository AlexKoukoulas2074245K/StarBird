///------------------------------------------------------------------------------------------------
///  WaveIntroGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "WaveIntroGameState.h"
#include "FightingWaveGameState.h"
#include "../Scene.h"
#include "../LevelUpdater.h"
#include "../SceneObjectConstants.h"
#include "../GameObjectConstants.h"
#include "../datarepos/FontRepository.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId WaveIntroGameState::STATE_NAME("WaveIntroGameState");

///------------------------------------------------------------------------------------------------

void WaveIntroGameState::Initialize()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    SceneObject waveTextSO;
    waveTextSO.mCustomPosition = game_object_constants::WAVE_INTRO_TEXT_INIT_POS;
    waveTextSO.mCustomScale = game_object_constants::WAVE_INTRO_TEXT_SCALE;
    waveTextSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
    waveTextSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
    waveTextSO.mTextureResourceId = FontRepository::GetInstance().GetFont(strutils::StringId("font"))->get().mFontTextureResourceId;
    waveTextSO.mFontName = strutils::StringId("font");
    waveTextSO.mSceneObjectType = SceneObjectType::GUIObject;
    waveTextSO.mNameTag = scene_object_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME;
    waveTextSO.mText = "WAVE " + std::to_string(mLevelUpdater->GetCurrentWaveNumber() + 1);
    waveTextSO.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene->AddSceneObject(std::move(waveTextSO));
    
    mLevelUpdater->AddFlow(RepeatableFlow([&]()
    {
        Complete(FightingWaveGameState::STATE_NAME);
    }, game_object_constants::WAVE_INTRO_DURATION_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_object_constants::WAVE_INTRO_FLOW_NAME));
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective WaveIntroGameState::Update(const float dtMillis)
{
    auto waveTextIntroSoOpt = mScene->GetSceneObject(scene_object_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME);
    auto waveTextIntroFlowOpt = mLevelUpdater->GetFlow(game_object_constants::WAVE_INTRO_FLOW_NAME);
    
    if (waveTextIntroSoOpt && waveTextIntroFlowOpt)
    {
        auto& waveTextIntroSo = waveTextIntroSoOpt->get();
        auto& waveTextIntroFlow = waveTextIntroFlowOpt->get();
        
        const auto ticksLeft = waveTextIntroFlow.GetTicksLeft();
        const auto halfDuration = waveTextIntroFlow.GetDuration()/2;
        if (ticksLeft > halfDuration)
        {
            waveTextIntroSo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f - (ticksLeft - halfDuration)/halfDuration;
        }
        else
        {
            waveTextIntroSo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = ticksLeft/halfDuration;
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void WaveIntroGameState::Destroy()
{
    mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME);
}

///------------------------------------------------------------------------------------------------
