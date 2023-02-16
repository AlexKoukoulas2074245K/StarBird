///------------------------------------------------------------------------------------------------
///  FightingWaveGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FightingWaveGameState.h"
#include "UpgradeOverlayInGameState.h"
#include "WaveIntroGameState.h"
#include "../LevelUpdater.h"
#include "../GameSingletons.h"
#include "../PhysicsConstants.h"
#include "../Scene.h"
#include "../datarepos/ObjectTypeDefinitionRepository.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../resloading/TextureResource.h"

#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

const strutils::StringId FightingWaveGameState::STATE_NAME("FightingWaveGameState");

///------------------------------------------------------------------------------------------------

void FightingWaveGameState::Initialize()
{
    mLevelUpdater->AdvanceWave();
    
    auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    for (const auto& enemy: mLevelUpdater->GetCurrentLevelDefinition().mWaves[mLevelUpdater->GetCurrentWaveNumber()].mEnemies)
    {
        const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemy.mGameObjectEnemyType);
        if (!enemyDefOpt) continue;
        const auto& enemyDef = enemyDefOpt->get();
        
        SceneObject so;
        
        const auto& defaultAnim = enemyDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE);
        if (defaultAnim.mAnimationType == AnimationType::VARIABLE_TEXTURED)
        {
            assert(!defaultAnim.mVariableTextureResourceIds.empty());
            so.mTextureResourceId = defaultAnim.mVariableTextureResourceIds.at(math::RandomInt(0, static_cast<int>(defaultAnim.mVariableTextureResourceIds.size()) - 1));
        }
        else
        {
            so.mTextureResourceId = enemyDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId;
        }
        
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(enemy.mPosition.x, enemy.mPosition.y);
        b2Body* body = mBox2dWorld->CreateBody(&bodyDef);
        body->SetLinearDamping(enemyDef.mLinearDamping);
        
        b2PolygonShape dynamicBox;
        auto& enemyTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(so.mTextureResourceId);
        
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
    
        mLevelUpdater->AddWaveEnemy(nameTag);
        
        mScene->AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective FightingWaveGameState::Update(const float)
{
    if (mLevelUpdater->GetWaveEnemyCount() == 0)
    {
        if (GameSingletons::GetAvailableUpgrades().size() > 1)
        {
            Complete(UpgradeOverlayInGameState::STATE_NAME);
        }
        else
        {
            Complete(WaveIntroGameState::STATE_NAME);
        }
    }
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------
