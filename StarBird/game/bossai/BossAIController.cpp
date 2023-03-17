///------------------------------------------------------------------------------------------------
///  BossAIController.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "BossAIController.h"
#include "KathunBossAI.h"
#include "../../utils/OSMessageBox.h"

///------------------------------------------------------------------------------------------------

BossAIController::BossAIController(Scene& scene, LevelUpdater& levelUpdater, StateMachine& stateMachine, b2World& box2dWorld)
    : mScene(scene)
    , mLevelUpdater(levelUpdater)
    , mStateMachine(stateMachine)
    , mBox2dWorld(box2dWorld)
{
    RegisterBossAIs();
}

///------------------------------------------------------------------------------------------------

BossAIController::~BossAIController(){}

///------------------------------------------------------------------------------------------------

void BossAIController::UpdateBossAI(const strutils::StringId& bossName, const float dtMillis)
{
    auto targetAIIter = mBossAIs.find(bossName);
    if (targetAIIter != mBossAIs.cend())
    {
        targetAIIter->second->VUpdateBossAI(dtMillis);
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Unable to find target AI", "BossAI could not be found for boss: " + bossName.GetString());
    }
}

///------------------------------------------------------------------------------------------------

void BossAIController::RegisterBossAIs()
{
    mBossAIs[KathunBossAI::BOSS_NAME] = std::make_unique<KathunBossAI>(mScene, mLevelUpdater, mStateMachine, mBox2dWorld);
}

///------------------------------------------------------------------------------------------------
