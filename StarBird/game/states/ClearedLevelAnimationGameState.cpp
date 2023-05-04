///------------------------------------------------------------------------------------------------
///  ClearedLevelAnimationGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 07/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "ClearedLevelAnimationGameState.h"
#include "../GameConstants.h"
#include "../GameSingletons.h"
#include "../LevelUpdater.h"
#include "../PhysicsConstants.h"
#include "../Scene.h"
#include "../../utils/Logging.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId ClearedLevelAnimationGameState::STATE_NAME("ClearedLevelAnimationGameState");

static const float TRANSITION_Y_THRESHOLD = 14.0f;

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective ClearedLevelAnimationGameState::VUpdate(const float dtMillis)
{
    auto playerSoOpt = mScene->GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    if (playerSoOpt)
    {
        auto& playerSo = playerSoOpt->get();
        playerSo.mCustomDrivenMovement = true;
        
        auto playerFilter = playerSo.mBody->GetFixtureList()[0].GetFilterData();
        playerFilter.maskBits &= ~(physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
        
        playerSo.mBody->GetFixtureList()[0].SetFilterData(playerFilter);
        playerSo.mBody->SetLinearVelocity(b2Vec2(0.0f, game_constants::BASE_PLAYER_SPEED * GameSingletons::GetPlayerMovementSpeedStat() * dtMillis));
        
        if (playerSo.mBody->GetWorldCenter().y >= TRANSITION_Y_THRESHOLD)
        {
            if (mLevelUpdater->GetCurrentLevelDefinition().mWaves.back().mBossName.isEmpty())
            {
                mScene->ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            }
            else
            {
                mScene->ChangeScene(Scene::TransitionParameters(Scene::SceneType::CHEST_REWARD, "", true));
            }
            Complete();
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------
