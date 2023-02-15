///------------------------------------------------------------------------------------------------
///  LevelUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameObjectConstants.h"
#include "GameSingletons.h"
#include "InputContext.h"
#include "LevelUpdater.h"
#include "ObjectTypeDefinitionRepository.h"
#include "PhysicsConstants.h"
#include "PhysicsCollisionListener.h"
#include "Scene.h"
#include "SceneObjectConstants.h"

#include "dataloaders/UpgradesLoader.h"

#include "../utils/Logging.h"
#include "../utils/ObjectiveCUtils.h"
#include "../utils/OSMessageBox.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/ShaderResource.h"
#include "../resloading/TextureResource.h"


#include <algorithm>
#include <Box2D/Box2D.h>
#include <map>

///------------------------------------------------------------------------------------------------

LevelUpdater::LevelUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mUpgradesLogicHandler(scene, *this)
    , mCurrentWaveNumber(0)
    , mState(LevelState::WAVE_INTRO)
    , mAllowInputControl(false)
{
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::InitLevel(LevelDefinition&& levelDef)
{
    mLevel = levelDef;
    
    mFlows.emplace_back([&]()
    {
        auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
        bool hasBulletDamageUpgrade = equippedUpgrades.count(gameobject_constants::BULLET_DAMAGE_UGPRADE_NAME) != 0;
        bool hasDoubleBulletUpgrade = equippedUpgrades.count(gameobject_constants::DOUBLE_BULLET_UGPRADE_NAME) != 0;
        bool hasMirrorImageUpgrade = equippedUpgrades.count(gameobject_constants::MIRROR_IMAGE_UGPRADE_NAME) != 0;
        
        auto playerOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerOpt)
        {
            if (hasDoubleBulletUpgrade)
            {
                auto bulletPosition = math::Box2dVec2ToGlmVec3(playerOpt->get().mBody->GetWorldCenter());
                
                // Left Bullet
                bulletPosition.x -= gameobject_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_PLAYER_BULLET_TYPE : gameobject_constants::PLAYER_BULLET_TYPE, bulletPosition);
                
                // Right Bullet
                bulletPosition.x += 2 * gameobject_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_PLAYER_BULLET_TYPE : gameobject_constants::PLAYER_BULLET_TYPE, bulletPosition);
                
                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    
                    if (leftMirrorImageSoOpt)
                    {
                        auto bulletPosition = leftMirrorImageSoOpt->get().mCustomPosition;
                        bulletPosition.x -= gameobject_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : gameobject_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                        
                        bulletPosition.x += 2 * gameobject_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : gameobject_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                    }
                    
                    if (rightMirrorImageSoOpt)
                    {
                        auto bulletPosition = rightMirrorImageSoOpt->get().mCustomPosition;
                        bulletPosition.x -= gameobject_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : gameobject_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                        
                        bulletPosition.x += 2 * gameobject_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : gameobject_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                    }
                }
            }
            else
            {
                CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_PLAYER_BULLET_TYPE : gameobject_constants::PLAYER_BULLET_TYPE, math::Box2dVec2ToGlmVec3( playerOpt->get().mBody->GetWorldCenter()));
                
                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    
                    if (leftMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : gameobject_constants::MIRROR_IMAGE_BULLET_TYPE, leftMirrorImageSoOpt->get().mCustomPosition);
                    }
                    
                    if (rightMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? gameobject_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : gameobject_constants::MIRROR_IMAGE_BULLET_TYPE, rightMirrorImageSoOpt->get().mCustomPosition);
                    }
                }
            }
        }
    }, gameobject_constants::PLAYER_BULLET_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::REPEAT, gameobject_constants::PLAYER_BULLET_FLOW_NAME);
    
    static PhysicsCollisionListener collisionListener;
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_CATEGORY_BIT, physics_constants::PLAYER_BULLET_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        strutils::StringId enemyBodyAddressTag;
        enemyBodyAddressTag.fromAddress(firstBody);
        auto enemySceneObjectOpt = mScene.GetSceneObject(enemyBodyAddressTag);
        
        strutils::StringId bulletAddressTag;
        bulletAddressTag.fromAddress(secondBody);
        auto bulletSceneObjectOpt = mScene.GetSceneObject(bulletAddressTag);
        
        if (enemySceneObjectOpt && bulletSceneObjectOpt)
        {
            auto& enemySO = enemySceneObjectOpt->get();
            auto& bulletSO = bulletSceneObjectOpt->get();
            
            auto enemySceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName)->get();
            auto bulletSceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(bulletSO.mObjectFamilyTypeName)->get();
            
            enemySO.mHealth -= bulletSceneObjectTypeDef.mDamage;
            
            if (enemySO.mHealth <= 0)
            {
                enemySO.mStateName = strutils::StringId("dying");
                enemySO.mUseBodyForRendering = false;
                enemySO.mCustomPosition.x = firstBody->GetWorldCenter().x;
                enemySO.mCustomPosition.y = firstBody->GetWorldCenter().y;
                
                auto filter = firstBody->GetFixtureList()[0].GetFilterData();
                filter.maskBits = 0;
                firstBody->GetFixtureList()[0].SetFilterData(filter);
                
                UpdateAnimation(enemySO, enemySceneObjectTypeDef, 0.0f);
                
                const auto& currentAnim = enemySceneObjectTypeDef.mAnimations.at(enemySO.mStateName);
                
                mFlows.emplace_back([=]()
                {
                    mWaveEnemies.erase(enemyBodyAddressTag);
                    mScene.RemoveAllSceneObjectsWithNameTag(enemyBodyAddressTag);
                }, currentAnim.mDuration, RepeatableFlow::RepeatPolicy::ONCE);
            }
            
            // Erase bullet collision mask so that it doesn't also contribute to other
            // enemy damage until it is removed from b2World
            auto bulletFilter = secondBody->GetFixtureList()[0].GetFilterData();
            bulletFilter.maskBits = 0;
            secondBody->GetFixtureList()[0].SetFilterData(bulletFilter);

            mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
        }
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_CATEGORY_BIT, physics_constants::ENEMY_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        auto iter = std::find_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
        {
            return flow.GetName() == gameobject_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME;
        });
        if (iter != mFlows.end()) return;
        
        auto playerSceneObjectOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
        strutils::StringId enemyBodyAddressTag;
        enemyBodyAddressTag.fromAddress(secondBody);
        auto enemySceneObjectOpt = mScene.GetSceneObject(enemyBodyAddressTag);
        
        if (playerSceneObjectOpt && enemySceneObjectOpt)
        {
            auto& playerSO = playerSceneObjectOpt->get();
            auto& enemySO = enemySceneObjectOpt->get();
            
            auto enemySceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName)->get();
            
            playerSO.mHealth -= enemySceneObjectTypeDef.mDamage;
            objectiveC_utils::Vibrate();
            
            mFlows.emplace_back([]()
            {
            }, gameobject_constants::PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, gameobject_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME);
            
            if (playerSO.mHealth <= 0)
            {
                playerSO.mStateName = strutils::StringId("dying");
                playerSO.mUseBodyForRendering = false;
                playerSO.mCustomPosition.x = firstBody->GetWorldCenter().x;
                playerSO.mCustomPosition.y = firstBody->GetWorldCenter().y;
                
                auto filter = firstBody->GetFixtureList()[0].GetFilterData();
                filter.maskBits = 0;
                firstBody->GetFixtureList()[0].SetFilterData(filter);
                
                UpdateAnimation(playerSO, ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(playerSO.mObjectFamilyTypeName)->get(), 0.0f);
                
                const auto& currentAnim = enemySceneObjectTypeDef.mAnimations.at(playerSO.mStateName);
                
                mFlows.emplace_back([=]()
                {
                    mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
                }, currentAnim.mDuration, RepeatableFlow::RepeatPolicy::ONCE);
            }
        }
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_BULLET_CATEGORY_BIT, physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        strutils::StringId bulletAddressTag;
        bulletAddressTag.fromAddress(firstBody);
        mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
    });

    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_CATEGORY_BIT, physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        strutils::StringId enemyAddressTag;
        enemyAddressTag.fromAddress(firstBody);
        mScene.RemoveAllSceneObjectsWithNameTag(enemyAddressTag);
        mWaveEnemies.erase(enemyAddressTag);
    });

    mBox2dWorld.SetContactListener(&collisionListener);
    
    UpgradesLoader loader;
    GameSingletons::SetAvailableUpgrades(loader.LoadAllUpgrades());
    
    mState = LevelState::WAVE_INTRO;
    mCurrentWaveNumber = 0;
    CreateWaveIntro();
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    auto stateMachineUpdateResult = UpdateStateMachine(dtMillis);
    if (stateMachineUpdateResult == StateMachineUpdateResult::BLOCK_UPDATE)
    {
        OnBlockedUpdate();
        return;
    }
    
    auto joystickSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    auto playerSO = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (joystickSO && joystickBoundsSO)
    {
        joystickSO->get().mInvisible = true;
        joystickBoundsSO->get().mInvisible = true;
    }
    
    for (auto& sceneObject: sceneObjects)
    {
        sceneObject.mShaderBoolUniformValues[sceneobject_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = false;
        
        // Check if this scene object has a respective family object definition
        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject.mObjectFamilyTypeName);
        if (sceneObjectTypeDefOpt)
        {
            // Update movement
            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
            switch (sceneObjectTypeDef.mMovementControllerPattern)
            {
                case MovementControllerPattern::CUSTOM_VELOCITY:
                {
                    sceneObject.mBody->SetLinearVelocity(b2Vec2(sceneObjectTypeDef.mCustomLinearVelocity.x, sceneObjectTypeDef.mCustomLinearVelocity.y));
                } break;
                    
                case MovementControllerPattern::CHASING_PLAYER:
                {
                    if (playerSO)
                    {
                        b2Vec2 toAttractionPoint = playerSO->get().mBody->GetWorldCenter() - sceneObject.mBody->GetWorldCenter();
                    
                        toAttractionPoint.Normalize();
                        toAttractionPoint.x *= dtMillis * sceneObjectTypeDef.mSpeed;
                        toAttractionPoint.y *= dtMillis * sceneObjectTypeDef.mSpeed;
                        sceneObject.mBody->ApplyForceToCenter(toAttractionPoint, true);
                    }
                } break;
                    
                case MovementControllerPattern::INPUT_CONTROLLED:
                {
                    UpdateInputControlledSceneObject(sceneObject, sceneObjectTypeDef, dtMillis);
                } break;
                    
                default: break;
            }
            
            // Update animation
            UpdateAnimation(sceneObject, sceneObjectTypeDef, dtMillis);
        }
    }
    
    mUpgradesLogicHandler.OnUpdate(dtMillis);
    UpdateHealthBars(dtMillis);
    UpdateBackground(dtMillis);
    UpdateFlows(dtMillis);
}

///------------------------------------------------------------------------------------------------

size_t LevelUpdater::GetWaveEnemyCount() const
{
    return mWaveEnemies.size();
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<RepeatableFlow>> LevelUpdater::GetFlow(const strutils::StringId& flowName)
{
    auto findIter = std::find_if(mFlows.begin(), mFlows.end(), [&](const RepeatableFlow& flow)
    {
        return flow.GetName() == flowName;
    });
    
    if (findIter != mFlows.end())
    {
        return std::optional<std::reference_wrapper<RepeatableFlow>>{*findIter};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

LevelUpdater::StateMachineUpdateResult LevelUpdater::UpdateStateMachine(const float dtMillis)
{
    static float tween = 0.0f;
    
    switch (mState)
    {
        case LevelState::WAVE_INTRO:
        {
            auto waveTextIntroSoOpt = mScene.GetSceneObject(sceneobject_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME);
            auto waveTextIntroFlowOpt = GetFlow(gameobject_constants::WAVE_INTRO_FLOW_NAME);
            
            if (waveTextIntroSoOpt && waveTextIntroFlowOpt)
            {
                auto& waveTextIntroSo = waveTextIntroSoOpt->get();
                auto& waveTextIntroFlow = waveTextIntroFlowOpt->get();
                
                const auto ticksLeft = waveTextIntroFlow.GetTicksLeft();
                const auto halfDuration = waveTextIntroFlow.GetDuration()/2;
                if (ticksLeft > halfDuration)
                {
                    waveTextIntroSo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f - (ticksLeft - halfDuration)/halfDuration;
                }
                else
                {
                    waveTextIntroSo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = ticksLeft/halfDuration;
                }
            }
        } break;
            
        case LevelState::FIGHTING_WAVE:
        {
            if (mWaveEnemies.size() == 0)
            {
                if (GameSingletons::GetAvailableUpgrades().size() > 1)
                {
                    mState = LevelState::UPGRADE_OVERLAY_IN;
                    CreateUpgradeSceneObjects();
                    tween = 0.0f;
                }
                else
                {
                    mState = LevelState::WAVE_INTRO;
                    mCurrentWaveNumber++;
                    CreateWaveIntro();
                }
            }
        } break;
        
        case LevelState::UPGRADE_OVERLAY_IN:
        {
            auto upgradeOverlaySoOpt = mScene.GetSceneObject(sceneobject_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
            if (upgradeOverlaySoOpt)
            {
                auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
                auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME];
                upgradeOverlayAlpha += dtMillis * gameobject_constants::OVERLAY_DARKENING_SPEED;
                if (upgradeOverlayAlpha >= gameobject_constants::UPGRADE_OVERLAY_MAX_ALPHA)
                {
                    upgradeOverlayAlpha = gameobject_constants::UPGRADE_OVERLAY_MAX_ALPHA;
                    mState = LevelState::UPGRADE;
                }
            }
            return StateMachineUpdateResult::BLOCK_UPDATE;
        } break;
            
        case LevelState::UPGRADE:
        {
            tween = math::Min(1.0f, tween + dtMillis * gameobject_constants::UPGRADE_MOVEMENT_SPEED);
            float perc = math::Min(1.0f, math::TweenValue(tween, math::BounceFunction, math::TweeningMode::EASE_IN));
            
            auto leftUpgradeContainerSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
            auto rightUpgradeContainerSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
            auto leftUpgradeSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
            auto rightUpgradeSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
            
            if (leftUpgradeContainerSoOpt)
            {
                leftUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * gameobject_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
            }
            
            if (rightUpgradeContainerSoOpt)
            {
                rightUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS.x + perc * gameobject_constants::RIGHT_UPGRADE_CONTAINER_TARGET_POS.x;
            }
            
            if (leftUpgradeSoOpt)
            {
                leftUpgradeSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::LEFT_UPGRADE_INIT_POS.x + perc * gameobject_constants::LEFT_UPGRADE_TARGET_POS.x;
            }
            
            if (rightUpgradeSoOpt)
            {
                rightUpgradeSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::RIGHT_UPGRADE_INIT_POS.x + perc * gameobject_constants::RIGHT_UPGRADE_TARGET_POS.x;
            }
            
            auto selectedUpgradeName = TestForUpgradeSelected();
            if (selectedUpgradeName != strutils::StringId())
            {
                auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
                auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
                
                if (selectedUpgradeName == sceneobject_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME)
                {
                    mUpgradesLogicHandler.OnUpgradeEquipped(mUpgradeSelection.first.mUpgradeName);
                    equippedUpgrades[mUpgradeSelection.first.mUpgradeName] = mUpgradeSelection.first;
                    availableUpgrades[mUpgradeSelection.second.mUpgradeName] = mUpgradeSelection.second;
                }
                else
                {
                    mUpgradesLogicHandler.OnUpgradeEquipped(mUpgradeSelection.second.mUpgradeName);
                    equippedUpgrades[mUpgradeSelection.second.mUpgradeName] = mUpgradeSelection.second;
                    availableUpgrades[mUpgradeSelection.first.mUpgradeName] = mUpgradeSelection.first;
                }
                
                mState = LevelState::UPGRADE_OVERLAY_OUT;
            }
            
            return StateMachineUpdateResult::BLOCK_UPDATE;
        } break;
            
        case LevelState::UPGRADE_OVERLAY_OUT:
        {
            tween = math::Max(0.0f, tween - dtMillis * gameobject_constants::UPGRADE_MOVEMENT_SPEED);
            float perc = math::Max(0.0f, math::TweenValue(tween, math::QuadFunction, math::TweeningMode::EASE_OUT));
            
            auto upgradeOverlaySoOpt = mScene.GetSceneObject(sceneobject_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
            auto leftUpgradeContainerSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
            auto rightUpgradeContainerSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
            auto leftUpgradeSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
            auto rightUpgradeSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
            
            if (leftUpgradeContainerSoOpt)
            {
                leftUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * gameobject_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
            }
            
            if (rightUpgradeContainerSoOpt)
            {
                rightUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS.x + perc * gameobject_constants::RIGHT_UPGRADE_CONTAINER_TARGET_POS.x;
            }
            
            if (leftUpgradeSoOpt)
            {
                leftUpgradeSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::LEFT_UPGRADE_INIT_POS.x + perc * gameobject_constants::LEFT_UPGRADE_TARGET_POS.x;
            }
            
            if (rightUpgradeSoOpt)
            {
                rightUpgradeSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::RIGHT_UPGRADE_INIT_POS.x + perc * gameobject_constants::RIGHT_UPGRADE_TARGET_POS.x;
            }
            
            if (upgradeOverlaySoOpt)
            {
                auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
                auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME];
                upgradeOverlayAlpha -= dtMillis * gameobject_constants::OVERLAY_DARKENING_SPEED;
                if (upgradeOverlayAlpha <= 0)
                {
                    mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
                    mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
                    mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
                    mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
                    mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
                    upgradeOverlayAlpha = 0;
                    mState = LevelState::WAVE_INTRO;
                    mCurrentWaveNumber++;
                    CreateWaveIntro();
                }
            }
            
            return StateMachineUpdateResult::BLOCK_UPDATE;
        } break;
            
        default: break;
    }
    
    return StateMachineUpdateResult::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateAnimation(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
{
    const auto& currentAnim = sceneObjectTypeDef.mAnimations.at(sceneObject.mStateName);
    const auto& currentTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(currentAnim.mTextureResourceId);

    if (currentTexture.GetSheetMetadata())
    {
        const auto& sheetMetaDataCurrentRow = currentTexture.GetSheetMetadata()->mRowMetadata[currentAnim.mTextureSheetRow];
        const auto animFrameTime = currentAnim.mDuration/sheetMetaDataCurrentRow.mColMetadata.size();
        
        if (currentAnim.mDuration > 0.0f)
        {
            sceneObject.mAnimationTime += dtMilis;
            if (sceneObject.mAnimationTime >= animFrameTime)
            {
                sceneObject.mAnimationTime = 0.0f;
                sceneObject.mAnimationIndex = (sceneObject.mAnimationIndex + 1) % sheetMetaDataCurrentRow.mColMetadata.size();
            }
        }
        sceneObject.mShaderBoolUniformValues[sceneobject_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MIN_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).minU;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MIN_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).minV;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MAX_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).maxU;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MAX_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).maxV;
        
        sceneObject.mCustomScale = glm::vec3(currentAnim.mScale, currentAnim.mScale, 1.0f);
        sceneObject.mTextureResourceId = currentAnim.mTextureResourceId;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    
    auto joystickSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    const auto& inputContext = GameSingletons::GetInputContext();
    
    switch (inputContext.mEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mCustomPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
                 joystickBoundsSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_Z;
                 
                 joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition;
                 joystickSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_BOUNDS_Z;

                 mAllowInputControl = true;
             }
        } break;
            
        case SDL_FINGERUP:
        {
            sceneObject.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        } break;
            
        case SDL_FINGERMOTION:
        {
            if (joystickBoundsSO && joystickSO && mAllowInputControl)
            {
                auto motionVec = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix()) - joystickBoundsSO->get().mCustomPosition;

                glm::vec3 norm = glm::normalize(motionVec);
                if (glm::length(motionVec) > glm::length(norm))
                {
                    motionVec = norm;
                }
                
                joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition + motionVec;
                joystickSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_Z;
                
                motionVec.x *= sceneObjectTypeDef.mSpeed * dtMilis;
                motionVec.y *= sceneObjectTypeDef.mSpeed * dtMilis;

                sceneObject.mBody->SetLinearVelocity(b2Vec2(motionVec.x, motionVec.y));
            }
            
        } break;
            
        default: break;
    }
    
    if (joystickSO && joystickBoundsSO && mAllowInputControl)
    {
       joystickSO->get().mInvisible = inputContext.mEventType == SDL_FINGERUP;
       joystickBoundsSO->get().mInvisible = inputContext.mEventType == SDL_FINGERUP;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateHealthBars(const float dtMillis)
{
    auto playerSoOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerHealthBarFrameSoOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto playerHealthBarSoOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME);
    
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
        
        healthBarSo.mCustomPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + gameobject_constants::HEALTH_BAR_POSITION_OFFSET;
        healthBarSo.mCustomPosition.z = gameobject_constants::PLAYER_HEALTH_BAR_Z;
        
        healthBarFrameSo.mCustomPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + gameobject_constants::HEALTH_BAR_POSITION_OFFSET;
        healthBarFrameSo.mCustomPosition.z = gameobject_constants::PLAYER_HEALTH_BAR_Z;
        
        float healthPerc = playerSo.mHealth/static_cast<float>(playerObjectDef.mHealth);
        
        healthBarSo.mCustomScale.x = gameobject_constants::HEALTH_BAR_SCALE.x * healthPerc;
        healthBarSo.mCustomPosition.x -= (1.0f - healthPerc)/2.0f * gameobject_constants::HEALTH_BAR_SCALE.x;
    }
    else
    {
        playerHealthBarFrameSoOpt->get().mInvisible = true;
        playerHealthBarSoOpt->get().mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateBackground(const float dtMillis)
{
    static float msAccum = 0.0f;
    msAccum += dtMillis * gameobject_constants::BACKGROUND_SPEED;
    msAccum = std::fmod(msAccum, 1.0f);
    
    auto bgSO = mScene.GetSceneObject(sceneobject_constants::BACKGROUND_SCENE_OBJECT_NAME);
    if (bgSO)
    {
       bgSO->get().mShaderFloatUniformValues[sceneobject_constants::TEXTURE_OFFSET_UNIFORM_NAME] = -msAccum;
    }
    
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateFlows(const float dtMillis)
{
    for (auto& flow: mFlows)
    {
        flow.Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::OnBlockedUpdate()
{
    auto joystickSoOpt = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSOopt = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    auto playerSoOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (playerSoOpt)
    {
        playerSoOpt->get().mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
    }
    
    if (joystickSoOpt)
    {
        joystickSoOpt->get().mInvisible = true;
    }
    
    if (joystickBoundsSOopt)
    {
        joystickBoundsSOopt->get().mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateWaveIntro()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    SceneObject waveTextSO;
    waveTextSO.mCustomPosition = gameobject_constants::WAVE_INTRO_TEXT_INIT_POS;
    waveTextSO.mCustomScale = gameobject_constants::WAVE_INTRO_TEXT_SCALE;
    waveTextSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
    waveTextSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
    waveTextSO.mTextureResourceId = FontRepository::GetInstance().GetFont(strutils::StringId("font"))->get().mFontTextureResourceId;
    waveTextSO.mFontName = strutils::StringId("font");
    waveTextSO.mSceneObjectType = SceneObjectType::GUIObject;
    waveTextSO.mNameTag = sceneobject_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME;
    waveTextSO.mText = "WAVE " + std::to_string(mCurrentWaveNumber + 1);
    waveTextSO.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene.AddSceneObject(std::move(waveTextSO));
    
    mFlows.emplace_back([&]()
    {
        mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME);
        mState = LevelState::FIGHTING_WAVE;
        CreateWave();
    }, gameobject_constants::WAVE_INTRO_DURATION_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, gameobject_constants::WAVE_INTRO_FLOW_NAME);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateWave()
{
    auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    for (const auto& enemy: mLevel.mWaves[mCurrentWaveNumber].mEnemies)
    {
        const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemy.mGameObjectEnemyType);
        if (!enemyDefOpt) continue;
        const auto& enemyDef = enemyDefOpt->get();
        
        SceneObject so;
        
        const auto& variableTextures = enemyDef.mAnimations.at(sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE).mVariableTextureResourceIds;
        if (variableTextures.size() > 0)
        {
            so.mTextureResourceId = variableTextures.at(math::RandomInt(0, static_cast<int>(variableTextures.size()) - 1));
        }
        else
        {
            so.mTextureResourceId = enemyDef.mAnimations.at(sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId;
        }
        
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(enemy.mPosition.x, enemy.mPosition.y);
        b2Body* body = mBox2dWorld.CreateBody(&bodyDef);
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
        
        mWaveEnemies.insert(nameTag);
        
        so.mNameTag = nameTag;
        mScene.AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateUpgradeSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
        overlaySo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::UPGRADE_OVERLAY_TEXTURE_FILE_NAME);
        overlaySo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mCustomScale = gameobject_constants::UPGRADE_OVERLAY_SCALE;
        overlaySo.mCustomPosition = gameobject_constants::UPGRADE_OVERLAY_POSITION;
        overlaySo.mNameTag = sceneobject_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME;
        overlaySo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mScene.AddSceneObject(std::move(overlaySo));
    }
    
    // Left Upgrade Container
    {
        SceneObject leftUpgradeContainerSO;
        leftUpgradeContainerSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        leftUpgradeContainerSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME);
        leftUpgradeContainerSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        leftUpgradeContainerSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeContainerSO.mCustomScale = gameobject_constants::LEFT_UPGRADE_CONTAINER_SCALE;
        leftUpgradeContainerSO.mCustomPosition = gameobject_constants::LEFT_UPGRADE_CONTAINER_INIT_POS;
        leftUpgradeContainerSO.mNameTag = sceneobject_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(leftUpgradeContainerSO));
    }
    
    // Left Upgrade
    {
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        auto upgradesIter = availableUpgrades.begin();
        std::advance(upgradesIter, math::RandomInt(0, static_cast<int>(availableUpgrades.size() - 1)));
        auto& upgrade = upgradesIter->second;
        
        SceneObject leftUpgradeSO;
        leftUpgradeSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        leftUpgradeSO.mTextureResourceId = upgrade.mTextureResourceId;
        leftUpgradeSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        leftUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeSO.mCustomScale = gameobject_constants::LEFT_UPGRADE_SCALE;
        leftUpgradeSO.mCustomPosition = gameobject_constants::LEFT_UPGRADE_INIT_POS;
        leftUpgradeSO.mNameTag = sceneobject_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(leftUpgradeSO));
        
        mUpgradeSelection.first = upgrade;
        availableUpgrades.erase(upgrade.mUpgradeName);
    }
    
    // Right Upgrade Container
    {
        SceneObject rightUpgradeContainerSO;
        rightUpgradeContainerSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        rightUpgradeContainerSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME);
        rightUpgradeContainerSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        rightUpgradeContainerSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeContainerSO.mCustomScale = gameobject_constants::RIGHT_UPGRADE_CONTAINER_SCALE;
        rightUpgradeContainerSO.mCustomPosition = gameobject_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS;
        rightUpgradeContainerSO.mNameTag = sceneobject_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(rightUpgradeContainerSO));
    }
    
    // Right Upgrade
    {
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        auto upgradesIter = availableUpgrades.begin();
        std::advance(upgradesIter, math::RandomInt(0, static_cast<int>(availableUpgrades.size() - 1)));
        auto& upgrade = upgradesIter->second;
        
        SceneObject rightUpgradeSO;
        rightUpgradeSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        rightUpgradeSO.mTextureResourceId = upgrade.mTextureResourceId;
        rightUpgradeSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        rightUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeSO.mCustomScale = gameobject_constants::RIGHT_UPGRADE_SCALE;
        rightUpgradeSO.mCustomPosition = gameobject_constants::RIGHT_UPGRADE_INIT_POS;
        rightUpgradeSO.mNameTag = sceneobject_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(rightUpgradeSO));
        
        mUpgradeSelection.second = upgrade;
        availableUpgrades.erase(upgrade.mUpgradeName);
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateBulletAtPosition(const strutils::StringId& bulletType, const glm::vec3& position)
{
    auto bulletDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(bulletType);
    if (bulletDefOpt)
    {
        auto& bulletDef = bulletDefOpt->get();
        
        SceneObject so;
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.x = position.x;
        bodyDef.position.y = position.y;
        bodyDef.bullet =  true;
        b2Body* body = mBox2dWorld.CreateBody(&bodyDef);
        body->SetLinearVelocity(b2Vec2(bulletDef.mCustomLinearVelocity.x, bulletDef.mCustomLinearVelocity.y));
        b2PolygonShape dynamicBox;
        
        auto& bulletTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(bulletDef.mAnimations.at(sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId);
        float textureAspect = bulletTexture.GetDimensions().x/bulletTexture.GetDimensions().y;
        dynamicBox.SetAsBox(bulletDef.mSize, bulletDef.mSize/textureAspect);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = bulletDef.mDensity;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter = bulletDef.mContactFilter;
        
        body->CreateFixture(&fixtureDef);
        
        so.mBody = body;
        so.mCustomPosition.z = gameobject_constants::BULLET_Z;
        so.mShaderResourceId = bulletDef.mShaderResourceId;
        so.mTextureResourceId = bulletDef.mAnimations.at(sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId;
        so.mMeshResourceId = bulletDef.mMeshResourceId;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mNameTag.fromAddress(so.mBody);
        so.mUseBodyForRendering = true;
        so.mObjectFamilyTypeName = bulletType;
        
        mScene.AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

strutils::StringId LevelUpdater::TestForUpgradeSelected() const
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    const auto& inputContext = GameSingletons::GetInputContext();
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto leftUpgradeSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
        auto rightUpgradeSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
        
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        // Left Upgrade test
        if (leftUpgradeSoOpt)
        {
            auto& leftUpgrade = leftUpgradeSoOpt->get();
            auto upgradeRectBottomLeft = glm::vec2(leftUpgrade.mCustomPosition.x - leftUpgrade.mCustomScale.x/2, leftUpgrade.mCustomPosition.y - leftUpgrade.mCustomScale.y/2);
            auto upgradeRectTopRight = glm::vec2(leftUpgrade.mCustomPosition.x + leftUpgrade.mCustomScale.x/2, leftUpgrade.mCustomPosition.y + leftUpgrade.mCustomScale.y/2);
            
            if (math::IsPointInsideRectangle(upgradeRectBottomLeft, upgradeRectTopRight, touchPos))
            {
                return leftUpgrade.mNameTag;
            }
        }
        
        // Right Upgrade test
        if (rightUpgradeSoOpt)
        {
            auto& rightUpgrade = rightUpgradeSoOpt->get();
            auto upgradeRectBottomLeft = glm::vec2(rightUpgrade.mCustomPosition.x - rightUpgrade.mCustomScale.x/2, rightUpgrade.mCustomPosition.y - rightUpgrade.mCustomScale.y/2);
            auto upgradeRectTopRight = glm::vec2(rightUpgrade.mCustomPosition.x + rightUpgrade.mCustomScale.x/2, rightUpgrade.mCustomPosition.y + rightUpgrade.mCustomScale.y/2);
            
            if (math::IsPointInsideRectangle(upgradeRectBottomLeft, upgradeRectTopRight, touchPos))
            {
                return rightUpgrade.mNameTag;
            }
        }
    }
    
    return strutils::StringId();
}

///------------------------------------------------------------------------------------------------
