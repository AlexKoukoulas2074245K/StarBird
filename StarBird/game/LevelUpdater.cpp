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
#include "states/FightingWaveGameState.h"
#include "states/WaveIntroGameState.h"
#include "states/PauseMenuGameState.h"
#include "states/UpgradeSelectionGameState.h"

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
    , mStateMachine(scene, *this, mUpgradesLogicHandler, mBox2dWorld)
    , mCurrentWaveNumber(0)
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
        bool hasBulletDamageUpgrade = equippedUpgrades.count(game_object_constants::BULLET_DAMAGE_UGPRADE_NAME) != 0;
        bool hasDoubleBulletUpgrade = equippedUpgrades.count(game_object_constants::DOUBLE_BULLET_UGPRADE_NAME) != 0;
        bool hasMirrorImageUpgrade = equippedUpgrades.count(game_object_constants::MIRROR_IMAGE_UGPRADE_NAME) != 0;
        
        auto playerOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerOpt)
        {
            if (hasDoubleBulletUpgrade)
            {
                auto bulletPosition = math::Box2dVec2ToGlmVec3(playerOpt->get().mBody->GetWorldCenter());
                
                // Left Bullet
                bulletPosition.x -= game_object_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_PLAYER_BULLET_TYPE : game_object_constants::PLAYER_BULLET_TYPE, bulletPosition);
                
                // Right Bullet
                bulletPosition.x += 2 * game_object_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_PLAYER_BULLET_TYPE : game_object_constants::PLAYER_BULLET_TYPE, bulletPosition);
                
                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = mScene.GetSceneObject(scene_object_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = mScene.GetSceneObject(scene_object_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    
                    if (leftMirrorImageSoOpt)
                    {
                        auto bulletPosition = leftMirrorImageSoOpt->get().mCustomPosition;
                        bulletPosition.x -= game_object_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_object_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                        
                        bulletPosition.x += 2 * game_object_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_object_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                    }
                    
                    if (rightMirrorImageSoOpt)
                    {
                        auto bulletPosition = rightMirrorImageSoOpt->get().mCustomPosition;
                        bulletPosition.x -= game_object_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_object_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                        
                        bulletPosition.x += 2 * game_object_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_object_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                    }
                }
            }
            else
            {
                CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_PLAYER_BULLET_TYPE : game_object_constants::PLAYER_BULLET_TYPE, math::Box2dVec2ToGlmVec3( playerOpt->get().mBody->GetWorldCenter()));
                
                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = mScene.GetSceneObject(scene_object_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = mScene.GetSceneObject(scene_object_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    
                    if (leftMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_object_constants::MIRROR_IMAGE_BULLET_TYPE, leftMirrorImageSoOpt->get().mCustomPosition);
                    }
                    
                    if (rightMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_object_constants::MIRROR_IMAGE_BULLET_TYPE, rightMirrorImageSoOpt->get().mCustomPosition);
                    }
                }
            }
        }
    }, game_object_constants::PLAYER_BULLET_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::REPEAT, game_object_constants::PLAYER_BULLET_FLOW_NAME);
    
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
            return flow.GetName() == game_object_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME;
        });
        if (iter != mFlows.end()) return;
        
        auto playerSceneObjectOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
        strutils::StringId enemyBodyAddressTag;
        enemyBodyAddressTag.fromAddress(secondBody);
        auto enemySceneObjectOpt = mScene.GetSceneObject(enemyBodyAddressTag);
        
        if (playerSceneObjectOpt && enemySceneObjectOpt)
        {
            auto& playerSO = playerSceneObjectOpt->get();
            auto& enemySO = enemySceneObjectOpt->get();
            
            auto enemySceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName)->get();
            
            if (mScene.GetSceneObject(scene_object_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME))
            {
                mScene.RemoveAllSceneObjectsWithNameTag(scene_object_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
            }
            else
            {
                playerSO.mHealth -= enemySceneObjectTypeDef.mDamage;
                objectiveC_utils::Vibrate();
            }
            
            mFlows.emplace_back([]()
            {
            }, game_object_constants::PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_object_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME);
            
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
                    mScene.RemoveAllSceneObjectsWithNameTag(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
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
    
    mStateMachine.RegisterState<FightingWaveGameState>();
    mStateMachine.RegisterState<WaveIntroGameState>();
    mStateMachine.RegisterState<PauseMenuGameState>();
    mStateMachine.RegisterState<UpgradeSelectionGameState>();

    mCurrentWaveNumber = 0;
    mStateMachine.InitStateMachine(WaveIntroGameState::STATE_NAME);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::OnAppStateChange(Uint32 event)
{
    switch (event)
    {
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND:
        {
            if (mLastPostStateMachineUpdateDirective != PostStateUpdateDirective::BLOCK_UPDATE)
            {
                mStateMachine.PushState(PauseMenuGameState::STATE_NAME);
            }
        } break;
            
        case SDL_APP_WILLENTERFOREGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
        {
            
        } break;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    mLastPostStateMachineUpdateDirective = mStateMachine.Update(dtMillis);
    if (mLastPostStateMachineUpdateDirective == PostStateUpdateDirective::BLOCK_UPDATE)
    {
        OnBlockedUpdate();
        return;
    }
    
    auto joystickSO = mScene.GetSceneObject(scene_object_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(scene_object_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    auto playerSO = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (joystickSO && joystickBoundsSO)
    {
        joystickSO->get().mInvisible = true;
        joystickBoundsSO->get().mInvisible = true;
    }
    
    for (auto& sceneObject: sceneObjects)
    {
        sceneObject.mShaderBoolUniformValues[scene_object_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = false;
        
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
                    sceneObject.mBody->SetLinearVelocity(b2Vec2(sceneObjectTypeDef.mConstantLinearVelocity.x, sceneObjectTypeDef.mConstantLinearVelocity.y));
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
        }
        
        if (sceneObjectTypeDefOpt || sceneObject.mCustomAnimation)
        {
            UpdateAnimation(sceneObject, sceneObjectTypeDefOpt, dtMillis);
        }
    }
    
    mUpgradesLogicHandler.OnUpdate(dtMillis);
    UpdateHealthBars(dtMillis);
    UpdateBackground(dtMillis);
    UpdateFlows(dtMillis);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateAnimation(SceneObject& sceneObject, std::optional<std::reference_wrapper<const ObjectTypeDefinition>> sceneObjectTypeDefOpt, const float dtMillis)
{
    if (!sceneObjectTypeDefOpt && !sceneObject.mCustomAnimation) return;
    
    auto& currentAnim = sceneObject.mCustomAnimation ? sceneObject.mCustomAnimation.value() : sceneObjectTypeDefOpt->get().mAnimations.at(sceneObject.mStateName);

    switch(currentAnim.mAnimationType)
    {
        case AnimationType::MULTI_FRAME:
        {
            const auto& currentTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(currentAnim.mTextureResourceId);
            const auto& sheetMetaDataCurrentRow = currentTexture.GetSheetMetadata()->mRowMetadata[currentAnim.mTextureSheetRow];
            const auto animFrameTime = currentAnim.mDuration/sheetMetaDataCurrentRow.mColMetadata.size();
            
            if (currentAnim.mDuration > 0.0f)
            {
                sceneObject.mAnimationTime += dtMillis;
                if (sceneObject.mAnimationTime >= animFrameTime)
                {
                    sceneObject.mAnimationTime = 0.0f;
                    sceneObject.mAnimationIndex = (sceneObject.mAnimationIndex + 1) % sheetMetaDataCurrentRow.mColMetadata.size();
                }
            }
            sceneObject.mShaderBoolUniformValues[scene_object_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
            sceneObject.mShaderFloatUniformValues[scene_object_constants::MIN_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).minU;
            sceneObject.mShaderFloatUniformValues[scene_object_constants::MIN_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).minV;
            sceneObject.mShaderFloatUniformValues[scene_object_constants::MAX_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).maxU;
            sceneObject.mShaderFloatUniformValues[scene_object_constants::MAX_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).maxV;
            
            sceneObject.mCustomScale = glm::vec3(currentAnim.mScale, currentAnim.mScale, 1.0f);
            sceneObject.mTextureResourceId = currentAnim.mTextureResourceId;
        } break;
        
        case AnimationType::PULSING:
        {
            currentAnim.mAnimDtAccum += dtMillis * currentAnim.mAnimDtAccumSpeed;
            sceneObject.mCustomScale += math::Sinf(currentAnim.mAnimDtAccum) * currentAnim.mPulsingEnlargementFactor;
        } break;
            
        default: break;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::AdvanceWave()
{
    mCurrentWaveNumber++;
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::AddFlow(RepeatableFlow&& flow)
{
    mFlows.emplace_back(std::move(flow));
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::AddWaveEnemy(const strutils::StringId& enemyTag)
{
    mWaveEnemies.insert(enemyTag);
}

///------------------------------------------------------------------------------------------------

const LevelDefinition& LevelUpdater::GetCurrentLevelDefinition() const
{
    return mLevel;
}

///------------------------------------------------------------------------------------------------

size_t LevelUpdater::GetCurrentWaveNumber() const
{
    return mCurrentWaveNumber;
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

void LevelUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    
    auto joystickSO = mScene.GetSceneObject(scene_object_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(scene_object_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    const auto& inputContext = GameSingletons::GetInputContext();
    
    switch (inputContext.mEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mCustomPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
                 joystickBoundsSO->get().mCustomPosition.z = game_object_constants::JOYSTICK_Z;
                 
                 joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition;
                 joystickSO->get().mCustomPosition.z = game_object_constants::JOYSTICK_BOUNDS_Z;

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
                joystickSO->get().mCustomPosition.z = game_object_constants::JOYSTICK_Z;
                
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
    auto playerSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerHealthBarFrameSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto playerHealthBarSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME);
    
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
        
        healthBarSo.mCustomScale.x = game_object_constants::HEALTH_BAR_SCALE.x * healthPerc;
        healthBarSo.mCustomPosition.x -= (1.0f - healthPerc)/2.0f * game_object_constants::HEALTH_BAR_SCALE.x;
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
    msAccum += dtMillis * game_object_constants::BACKGROUND_SPEED;
    msAccum = std::fmod(msAccum, 1.0f);
    
    auto bgSO = mScene.GetSceneObject(scene_object_constants::BACKGROUND_SCENE_OBJECT_NAME);
    if (bgSO)
    {
       bgSO->get().mShaderFloatUniformValues[scene_object_constants::TEXTURE_OFFSET_UNIFORM_NAME] = -msAccum;
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
    auto joystickSoOpt = mScene.GetSceneObject(scene_object_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSOopt = mScene.GetSceneObject(scene_object_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    auto playerSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    
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
        body->SetLinearVelocity(b2Vec2(bulletDef.mConstantLinearVelocity.x, bulletDef.mConstantLinearVelocity.y));
        b2PolygonShape dynamicBox;
        
        auto& bulletTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(bulletDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId);
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
        so.mCustomPosition.z = game_object_constants::BULLET_Z;
        so.mShaderResourceId = bulletDef.mShaderResourceId;
        so.mTextureResourceId = bulletDef.mAnimations.at(scene_object_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId;
        so.mMeshResourceId = bulletDef.mMeshResourceId;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mNameTag.fromAddress(so.mBody);
        so.mUseBodyForRendering = true;
        so.mObjectFamilyTypeName = bulletType;
        
        mScene.AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------
