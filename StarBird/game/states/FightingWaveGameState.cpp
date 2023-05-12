///------------------------------------------------------------------------------------------------
///  FightingWaveGameState.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 16/02/2023
///------------------------------------------------------------------------------------------------

#include "FightingWaveGameState.h"
#include "WaveIntroGameState.h"

#include "../LevelUpdater.h"
#include "../GameSingletons.h"
#include "../GameConstants.h"
#include "../PersistenceUtils.h"
#include "../PhysicsConstants.h"
#include "../Scene.h"
#include "../SceneObjectUtils.h"
#include "../Sounds.h"
#include "../datarepos/ObjectTypeDefinitionRepository.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../resloading/MeshResource.h"
#include "../../resloading/TextureResource.h"
#include "../../utils/Logging.h"
#include "../../utils/ObjectiveCUtils.h"

#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

const strutils::StringId FightingWaveGameState::STATE_NAME("FightingWaveGameState");

static const float EXPLOSION_SPEED = 0.001f;
static const float EXPLOSION_FADE_OUT_ALPHA_SPEED = 0.00025f;

///------------------------------------------------------------------------------------------------

void FightingWaveGameState::VInitialize()
{
    mBossDeathAnimationActive = false;
    mPlayerDeathAnimationActive = false;
    GameSingletons::SetBossCurrentHealth(1.0f);
    
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
    
    if (!currentWave.mBossName.isEmpty())
    {
        objectiveC_utils::PlaySound(sounds::BOSS_INTRO_SFX);
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective FightingWaveGameState::VUpdate(const float dtMillis)
{
    if (mBossDeathAnimationActive)
    {
        std::unordered_set<strutils::StringId, strutils::StringIdHasher> enemyNamesToRemove;
        
        for (const auto& enemyName: mLevelUpdater->GetWaveEnemyNames())
        {
            auto enemySoOpt = mScene->GetSceneObject(enemyName);
            if (enemySoOpt && scene_object_utils::IsSceneObjectBossPart(*enemySoOpt))
            {
                auto& enemySo = enemySoOpt->get();
                
                UpdateExplodingSpecialEntity(dtMillis, enemySo);
                
                if (enemySo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] < 0.0f)
                {
                    enemyNamesToRemove.insert(enemyName);
                }
            }
        }
        
        for (const auto& enemyNameToRemove: enemyNamesToRemove)
        {
            mLevelUpdater->RemoveWaveEnemy(enemyNameToRemove);
        }
    }
    else if (!mLevelUpdater->GetCurrentLevelDefinition().mWaves.at(mLevelUpdater->GetCurrentWaveNumber()).mBossName.isEmpty())
    {
        if (GameSingletons::GetBossCurrentHealth() <= 0.0f)
        {
            auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
            
            std::unordered_set<strutils::StringId, strutils::StringIdHasher> enemyNamesToRemoveInstantly;
            
            for (const auto& enemyName: mLevelUpdater->GetWaveEnemyNames())
            {
                auto enemySoOpt = mScene->GetSceneObject(enemyName);
                if (enemySoOpt && !scene_object_utils::IsSceneObjectBossPart(*enemySoOpt))
                {
                    const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemySoOpt->get().mObjectFamilyTypeName);
                    if (!enemyDefOpt || enemyDefOpt->get().mAnimations.count(game_constants::DYING_SCENE_OBJECT_STATE) == 0)
                    {
                        enemyNamesToRemoveInstantly.insert(enemyName);
                    }
                    else
                    {
                        const auto& enemyDef = enemyDefOpt->get();
                        scene_object_utils::ChangeSceneObjectState(*enemySoOpt, enemyDef, game_constants::DYING_SCENE_OBJECT_STATE);
                        
                        auto enemyName = enemySoOpt->get().mName;
                        
                        mLevelUpdater->AddFlow(RepeatableFlow([this, enemyName]()
                        {
                            mLevelUpdater->RemoveWaveEnemy(enemyName);
                        }, enemySoOpt->get().mAnimation->VGetDurationMillis(), RepeatableFlow::RepeatPolicy::ONCE));
                    }
                }
                else if (enemySoOpt && scene_object_utils::IsSceneObjectBossPart(*enemySoOpt))
                {
                    auto& enemySo = enemySoOpt->get();
                    const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemySoOpt->get().mObjectFamilyTypeName);
                    
                    enemySo.mAnimation->ChangeShaderResourceId(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME));
                    enemySo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                    
                    
                    if (enemyDefOpt)
                    {
                        auto droppedCrystalsPosition = enemySo.mBody ? (math::Box2dVec2ToGlmVec3(enemySo.mBody->GetWorldCenter())  - enemySo.mBodyCustomOffset): enemySo.mPosition;
                        
                        mLevelUpdater->DropCrystals(droppedCrystalsPosition, 0.0f, enemyDefOpt->get().mCrystalYield);
                    }
                }
            }
            
            for (const auto& enemyNameToRemoveInstantly: enemyNamesToRemoveInstantly)
            {
                mLevelUpdater->RemoveWaveEnemy(enemyNameToRemoveInstantly);
            }
            
            auto playerSoOpt = mScene->GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
            if (playerSoOpt)
            {
                auto& playerSo = playerSoOpt->get();
                playerSo.mInvulnerable = true;
            }
            
            objectiveC_utils::PauseMusicOnly();
            objectiveC_utils::PlaySound(sounds::PLAYER_BOSS_EXPLOSION_SFX);
            mBossDeathAnimationActive = true;
        }
    }
    
    if (mPlayerDeathAnimationActive)
    {
        auto playerSoOpt = mScene->GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerSoOpt)
        {
            auto& playerSo = playerSoOpt->get();
            
            UpdateExplodingSpecialEntity(dtMillis, playerSo);
            
            if (playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] < 0.0f)
            {
                Complete();
                mScene->ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAIN_MENU, "", true));
            }
        }
    }
    else if (GameSingletons::GetPlayerCurrentHealth()/GameSingletons::GetPlayerMaxHealth() <= 0.0f)
    {
        auto playerSoOpt = mScene->GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerSoOpt)
        {
            auto& playerSo = playerSoOpt->get();
            
            playerSo.mInvulnerable = true;
            playerSo.mAnimation->ChangeShaderResourceId(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME));
            playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        }
        
        objectiveC_utils::PauseMusicOnly();
        objectiveC_utils::PlaySound(sounds::PLAYER_BOSS_EXPLOSION_SFX);
        mPlayerDeathAnimationActive = true;
        mScene->SetProgressResetFlag();
    }
    
    if (mLevelUpdater->GetWaveEnemyCount() == 0 && GameSingletons::GetPlayerCurrentHealth() > 0.0f)
    {
        mLevelUpdater->AdvanceWave();
        Complete(WaveIntroGameState::STATE_NAME);
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void FightingWaveGameState::UpdateExplodingSpecialEntity(const float dtMillis, SceneObject& sceneObject)
{
    auto& mesh =  resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(sceneObject.mAnimation->VGetCurrentMeshResourceId());
    
    mesh.ApplyDirectTransformToData([dtMillis](resources::MeshResource::MeshData& data)
    {
        for (int i = 0; i < data.mVertices.size(); ++i)
        {
            auto oldZ = data.mVertices[i].z;

            if (math::Abs(data.mNormals[i].z) > 0.8)
            {
                data.mVertices[i] += glm::normalize(data.mVertices[i]) * dtMillis * EXPLOSION_SPEED;
                data.mVertices[i + 1] += glm::normalize(data.mVertices[i]) * dtMillis * EXPLOSION_SPEED;
                data.mVertices[i + 2] += glm::normalize(data.mVertices[i]) * dtMillis * EXPLOSION_SPEED;

                i += 2;
            }
            else
            {
                data.mVertices[i] += glm::normalize(data.mNormals[i]) * dtMillis * EXPLOSION_SPEED;
            }

            data.mVertices[i].z = oldZ;
        }
    });
    
    auto alphaFadeOutSpeed = EXPLOSION_FADE_OUT_ALPHA_SPEED * dtMillis;
    if (sceneObject.mName == game_constants::PLAYER_SCENE_OBJECT_NAME)
    {
        alphaFadeOutSpeed *= 2.0f;
    }
    sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= alphaFadeOutSpeed;
}

///------------------------------------------------------------------------------------------------
