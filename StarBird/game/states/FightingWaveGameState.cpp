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
#include "../GameObjectConstants.h"
#include "../PhysicsConstants.h"
#include "../Scene.h"
#include "../datarepos/ObjectTypeDefinitionRepository.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../resloading/TextureResource.h"

#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

const strutils::StringId FightingWaveGameState::STATE_NAME("FightingWaveGameState");

///------------------------------------------------------------------------------------------------

FightingWaveGameState::FightingWaveGameState()
    : mAnimatedHealthBarPerc(1.0f)
{
}

///------------------------------------------------------------------------------------------------

void FightingWaveGameState::VInitialize()
{
    auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    for (const auto& enemy: mLevelUpdater->GetCurrentLevelDefinition().mWaves[mLevelUpdater->GetCurrentWaveNumber()].mEnemies)
    {
        const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemy.mGameObjectEnemyType);
        if (!enemyDefOpt) continue;
        const auto& enemyDef = enemyDefOpt->get();
        
        SceneObject so;
        
        so.mAnimation = enemyDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE)->VClone();
        
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(enemy.mPosition.x, enemy.mPosition.y);
        b2Body* body = mBox2dWorld->CreateBody(&bodyDef);
        body->SetLinearDamping(enemyDef.mLinearDamping);
        
        b2PolygonShape dynamicBox;
        auto& enemyTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(so.mAnimation->VGetCurrentTextureResourceId());
        
        float textureAspect = enemyTexture.GetDimensions().x/enemyTexture.GetDimensions().y;
        dynamicBox.SetAsBox(enemyDef.mSize, enemyDef.mSize/textureAspect);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = enemyDef.mDensity;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter = enemyDef.mContactFilter;
        fixtureDef.filter.maskBits &= (~physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
        fixtureDef.filter.maskBits &= (~physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT);
        body->CreateFixture(&fixtureDef);
    
        so.mObjectFamilyTypeName = enemy.mGameObjectEnemyType;
        so.mBody = body;
        so.mHealth = enemyDef.mHealth;
        so.mShaderResourceId = enemyDef.mShaderResourceId;
        so.mMeshResourceId = enemyDef.mMeshResourceId;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mCustomPosition.z = 0.0f;
        so.mUseBodyForRendering = true;
        
        strutils::StringId nameTag;
        nameTag.fromAddress(so.mBody);
        so.mNameTag = nameTag;
    
        if (!enemyDef.mProjectileType.isEmpty())
        {
            auto projectileFlowName = strutils::StringId(so.mNameTag.GetString() + game_object_constants::ENEMY_PROJECTILE_FLOW_POSTFIX);
            mLevelUpdater->AddFlow(RepeatableFlow([=]()
            {
                auto bulletDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemyDef.mProjectileType);
                auto sourceEnemySoOpt = mScene->GetSceneObject(nameTag);
                
                if (bulletDefOpt && sourceEnemySoOpt)
                {
                    auto& bulletDef = bulletDefOpt->get();
                    auto& sourceEnemySo = sourceEnemySoOpt->get();
                    
                    SceneObject so;
                    b2BodyDef bodyDef;
                    bodyDef.type = b2_dynamicBody;
                    bodyDef.position.x = sourceEnemySo.mBody->GetWorldCenter().x;
                    bodyDef.position.y = sourceEnemySo.mBody->GetWorldCenter().y;
                    bodyDef.bullet =  true;
                    b2Body* body = mBox2dWorld->CreateBody(&bodyDef);
                    body->SetLinearVelocity(b2Vec2(bulletDef.mConstantLinearVelocity.x, bulletDef.mConstantLinearVelocity.y));
                    b2PolygonShape dynamicBox;
                    
                    auto& bulletTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(bulletDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE)->VGetCurrentTextureResourceId());
                    float textureAspect = bulletTexture.GetDimensions().x/bulletTexture.GetDimensions().y;
                    dynamicBox.SetAsBox(bulletDef.mSize, bulletDef.mSize/textureAspect);
                    
                    b2FixtureDef fixtureDef;
                    fixtureDef.shape = &dynamicBox;
                    fixtureDef.density = bulletDef.mDensity;
                    fixtureDef.friction = 0.0f;
                    fixtureDef.restitution = 0.0f;
                    fixtureDef.filter = bulletDef.mContactFilter;
                    fixtureDef.filter.maskBits &= (~physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
                    fixtureDef.filter.maskBits &= (~physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT);
                    
                    body->CreateFixture(&fixtureDef);
                    
                    so.mBody = body;
                    so.mCustomPosition.z = game_object_constants::BULLET_Z;
                    so.mShaderResourceId = bulletDef.mShaderResourceId;
                    so.mAnimation = std::make_unique<SingleFrameAnimation>(bulletDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE)->VGetCurrentTextureResourceId());
                    so.mMeshResourceId = bulletDef.mMeshResourceId;
                    so.mSceneObjectType = SceneObjectType::WorldGameObject;
                    so.mNameTag.fromAddress(so.mBody);
                    so.mUseBodyForRendering = true;
                    so.mObjectFamilyTypeName = enemyDef.mProjectileType;
                    
                    strutils::StringId nameTag;
                    nameTag.fromAddress(so.mBody);
                    so.mNameTag = nameTag;
                    mLevelUpdater->AddWaveEnemy(so.mNameTag);
                    
                    mScene->AddSceneObject(std::move(so));
                }
            }, enemyDef.mShootingFrequencyMillis, RepeatableFlow::RepeatPolicy::REPEAT, projectileFlowName));
        }
                                   
        mLevelUpdater->AddWaveEnemy(nameTag);
        
        mScene->AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective FightingWaveGameState::VUpdate(const float dtMillis)
{
    UpdateHealthBars(dtMillis);
    
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

void FightingWaveGameState::UpdateHealthBars(const float dtMillis)
{
    auto playerSoOpt = mScene->GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerHealthBarFrameSoOpt = mScene->GetSceneObject(scene_object_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto playerHealthBarSoOpt = mScene->GetSceneObject(scene_object_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME);
    
    if (!playerHealthBarFrameSoOpt || !playerHealthBarSoOpt)
    {
        return;
    }
    
    if (playerSoOpt)
    {
        auto& playerSo = playerSoOpt->get();
        auto& healthBarFrameSo = playerHealthBarFrameSoOpt->get();
        auto& healthBarSo = playerHealthBarSoOpt->get();
        
        auto& playerObjectDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(playerSo.mObjectFamilyTypeName)->get();
        
        healthBarSo.mCustomPosition = game_object_constants::HEALTH_BAR_POSITION;
        healthBarSo.mCustomPosition.z = game_object_constants::PLAYER_HEALTH_BAR_Z;
        
        healthBarFrameSo.mCustomPosition = game_object_constants::HEALTH_BAR_POSITION;
        healthBarFrameSo.mCustomPosition.z = game_object_constants::PLAYER_HEALTH_BAR_Z;
        
        float healthPerc = playerSo.mHealth/static_cast<float>(playerObjectDef.mHealth);
        
        healthBarSo.mCustomScale.x = game_object_constants::HEALTH_BAR_SCALE.x * mAnimatedHealthBarPerc;
        healthBarSo.mCustomPosition.x -= (1.0f - mAnimatedHealthBarPerc)/2.0f * game_object_constants::HEALTH_BAR_SCALE.x;
        
        if (healthPerc < mAnimatedHealthBarPerc)
        {
            mAnimatedHealthBarPerc -= game_object_constants::HEALTH_LOST_SPEED * dtMillis;
        }
    }
    else
    {
        playerHealthBarFrameSoOpt->get().mInvisible = true;
        playerHealthBarSoOpt->get().mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

