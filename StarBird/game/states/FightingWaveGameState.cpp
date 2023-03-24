///------------------------------------------------------------------------------------------------
///  FightingWaveGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FightingWaveGameState.h"
#include "UpgradeSelectionGameState.h"
#include "WaveIntroGameState.h"

#include "../LevelUpdater.h"
#include "../GameSingletons.h"
#include "../GameConstants.h"
#include "../PhysicsConstants.h"
#include "../Scene.h"
#include "../SceneObjectUtils.h"
#include "../datarepos/ObjectTypeDefinitionRepository.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../resloading/TextureResource.h"
#include "../../utils/Logging.h"

#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

const strutils::StringId FightingWaveGameState::STATE_NAME("FightingWaveGameState");

///------------------------------------------------------------------------------------------------

void FightingWaveGameState::VInitialize()
{
    auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    const auto& currentWave = mLevelUpdater->GetCurrentLevelDefinition().mWaves[mLevelUpdater->GetCurrentWaveNumber()];
    for (const auto& enemy: currentWave.mEnemies)
    {
        const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemy.mGameObjectEnemyType);
        if (!enemyDefOpt) continue;
        const auto& enemyDef = enemyDefOpt->get();
        
        SceneObject so = scene_object_utils::CreateSceneObjectWithBody(enemyDef, enemy.mPosition, *mBox2dWorld, currentWave.mBossName.isEmpty() ? strutils::StringId() : enemyDef.mName);
        
        auto enemyName = so.mName;
        
        if (!enemyDef.mProjectileType.isEmpty())
        {
            auto projectileFlowName = strutils::StringId(so.mName.GetString() + game_constants::ENEMY_PROJECTILE_FLOW_POSTFIX);
            mLevelUpdater->AddFlow(RepeatableFlow([=]()
            {
                auto bulletDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemyDef.mProjectileType);
                auto sourceEnemySoOpt = mScene->GetSceneObject(enemyName);
                
                if (bulletDefOpt && sourceEnemySoOpt)
                {
                    auto& bulletDef = bulletDefOpt->get();
                    auto& sourceEnemySo = sourceEnemySoOpt->get();
                    
                    auto bulletPosition = math::Box2dVec2ToGlmVec3(sourceEnemySo.mBody->GetWorldCenter());
                    bulletPosition.z = game_constants::BULLET_Z;
                    SceneObject bulletSceneObject = scene_object_utils::CreateSceneObjectWithBody(bulletDef, bulletPosition, *mBox2dWorld);
                    
                    mLevelUpdater->AddWaveEnemy(bulletSceneObject.mName);
                    mScene->AddSceneObject(std::move(bulletSceneObject));
                }
                else
                {
                    Log(LogType::INFO, "Flow %s is dead", projectileFlowName.GetString().c_str());
                }
            }, enemyDef.mShootingFrequencyMillis, RepeatableFlow::RepeatPolicy::REPEAT, projectileFlowName));
        }
                                   
        mLevelUpdater->AddWaveEnemy(enemyName);
        
        mScene->AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective FightingWaveGameState::VUpdate(const float dtMillis)
{
    if (mLevelUpdater->GetWaveEnemyCount() == 0)
    {
        mLevelUpdater->AdvanceWave();
        
        if (GameSingletons::GetAvailableUpgrades().size() > 1)
        {
            Complete(UpgradeSelectionGameState::STATE_NAME);
        }
        else
        {
            Complete(WaveIntroGameState::STATE_NAME);
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

