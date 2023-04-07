///------------------------------------------------------------------------------------------------
///  ClearedLevelAnimationGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 07/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "ClearedLevelAnimationGameState.h"
#include "../GameConstants.h"
#include "../GameSingletons.h"
#include "../Scene.h"
#include "../../utils/Logging.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId ClearedLevelAnimationGameState::STATE_NAME("ClearedLevelAnimationGameState");

static const float TRANSITION_Y_THRESHOLD = 12.7f;

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective ClearedLevelAnimationGameState::VUpdate(const float dtMillis)
{
    auto playerSoOpt = mScene->GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    if (playerSoOpt)
    {
        auto& playerSo = playerSoOpt->get();
        playerSo.mCustomDrivenMovement = true;
        playerSo.mBody->SetLinearVelocity(b2Vec2(0.0f, game_constants::BASE_PLAYER_SPEED * GameSingletons::GetPlayerMovementSpeedStat() * dtMillis));
        
        if (playerSo.mBody->GetWorldCenter().y >= TRANSITION_Y_THRESHOLD)
        {
            mScene->ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            Complete();
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------
