///------------------------------------------------------------------------------------------------
///  LevelUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "InputContext.h"
#include "LevelUpdater.h"
#include "ObjectTypeDefinitionRepository.h"
#include "PhysicsConstants.h"
#include "PhysicsCollisionListener.h"
#include "Scene.h"
#include "SceneObjectUtils.h"

#include "dataloaders/UpgradesLoader.h"
#include "states/BossIntroGameState.h"
#include "states/DebugConsoleGameState.h"
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
#include <SDL.h>

///------------------------------------------------------------------------------------------------

LevelUpdater::LevelUpdater(Scene& scene, b2World& box2dWorld, LevelDefinition&& levelDef)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mUpgradesLogicHandler(scene, *this)
    , mStateMachine(&scene, this, &mUpgradesLogicHandler, &mBox2dWorld)
    , mBossAIController(scene, *this, mStateMachine, mBox2dWorld)
    , mCurrentWaveNumber(0)
    , mPlayerAnimatedHealthBarPerc(1.0f)
    , mBossAnimatedHealthBarPerc(0.0f)
    , mAllowInputControl(false)
    , mMovementRotationAllowed(false)
{
    mLevel = levelDef;
    
    mFlows.emplace_back([&]()
    {
        auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
        bool hasBulletDamageUpgrade = equippedUpgrades.count(game_constants::BULLET_DAMAGE_UGPRADE_NAME) != 0;
        bool hasDoubleBulletUpgrade = equippedUpgrades.count(game_constants::DOUBLE_BULLET_UGPRADE_NAME) != 0;
        bool hasMirrorImageUpgrade = equippedUpgrades.count(game_constants::MIRROR_IMAGE_UGPRADE_NAME) != 0;

        auto playerOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerOpt && playerOpt->get().mStateName == game_constants::DEFAULT_SCENE_OBJECT_STATE)
        {
            if (hasDoubleBulletUpgrade)
            {
                auto bulletPosition = math::Box2dVec2ToGlmVec3(playerOpt->get().mBody->GetWorldCenter());

                // Left Bullet
                bulletPosition.x -= game_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_PLAYER_BULLET_TYPE : game_constants::PLAYER_BULLET_TYPE, bulletPosition);

                // Right Bullet
                bulletPosition.x += 2 * game_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_PLAYER_BULLET_TYPE : game_constants::PLAYER_BULLET_TYPE, bulletPosition);

                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = mScene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = mScene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);

                    if (leftMirrorImageSoOpt)
                    {
                        auto bulletPosition = leftMirrorImageSoOpt->get().mPosition;
                        bulletPosition.x -= game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);

                        bulletPosition.x += 2 * game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                    }

                    if (rightMirrorImageSoOpt)
                    {
                        auto bulletPosition = rightMirrorImageSoOpt->get().mPosition;
                        bulletPosition.x -= game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);

                        bulletPosition.x += 2 * game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition);
                    }
                }
            }
            else
            {
                CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_PLAYER_BULLET_TYPE : game_constants::PLAYER_BULLET_TYPE, math::Box2dVec2ToGlmVec3( playerOpt->get().mBody->GetWorldCenter()));

                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = mScene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = mScene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);

                    if (leftMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_constants::MIRROR_IMAGE_BULLET_TYPE, leftMirrorImageSoOpt->get().mPosition);
                    }

                    if (rightMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(hasBulletDamageUpgrade ? game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE : game_constants::MIRROR_IMAGE_BULLET_TYPE, rightMirrorImageSoOpt->get().mPosition);
                    }
                }
            }
        }
    }, game_constants::PLAYER_BULLET_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::REPEAT, game_constants::PLAYER_BULLET_FLOW_NAME);

    static PhysicsCollisionListener collisionListener;
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_CATEGORY_BIT, physics_constants::PLAYER_BULLET_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        const auto& enemyName = *static_cast<strutils::StringId*>(firstBody->GetUserData());
        auto enemySceneObjectOpt = mScene.GetSceneObject(enemyName);
        
        const auto& bulletName = *static_cast<strutils::StringId*>(secondBody->GetUserData());
        auto bulletSceneObjectOpt = mScene.GetSceneObject(bulletName);
        
        if (enemySceneObjectOpt && bulletSceneObjectOpt)
        {
            auto& enemySO = enemySceneObjectOpt->get();
            auto& bulletSO = bulletSceneObjectOpt->get();
            
            auto enemySceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName)->get();
            auto bulletSceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(bulletSO.mObjectFamilyTypeName)->get();
            
            if (!enemySO.mInvulnerable)
            {
                if (scene_object_utils::IsSceneObjectBossPart(enemySO))
                {
                    GameSingletons::SetBossCurrentHealth(math::Max(0.0f, GameSingletons::GetBossCurrentHealth() - bulletSceneObjectTypeDef.mDamage));
                }
                else
                {
                    enemySO.mHealth -= bulletSceneObjectTypeDef.mDamage;
                }
                
                CreateTextOnDamage(enemySO.mName, math::Box2dVec2ToGlmVec3(enemySO.mBody->GetWorldCenter()), bulletSceneObjectTypeDef.mDamage);
            }
            
            if (enemySO.mHealth <= 0)
            {
                scene_object_utils::ChangeSceneObjectState(enemySO, enemySceneObjectTypeDef, strutils::StringId("dying"));
                
                mFlows.emplace_back([=]()
                {
                    RemoveWaveEnemy(enemyName);
                }, enemySO.mAnimation->VGetDuration(), RepeatableFlow::RepeatPolicy::ONCE);
                
                mActiveLightNames.insert(enemyName);
                mScene.GetLightRepository().AddLight(LightType::POINT_LIGHT, enemyName, game_constants::POINT_LIGHT_COLOR, enemySO.mPosition, game_constants::EXPLOSION_LIGHT_POWER);
            }
            
            // Erase bullet collision mask so that it doesn't also contribute to other
            // enemy damage until it is removed from b2World
            auto bulletFilter = secondBody->GetFixtureList()[0].GetFilterData();
            bulletFilter.maskBits = 0;
            secondBody->GetFixtureList()[0].SetFilterData(bulletFilter);

            mScene.RemoveAllSceneObjectsWithName(bulletName);
        }
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_CATEGORY_BIT, physics_constants::ENEMY_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        auto iter = std::find_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
        {
            return flow.GetName() == game_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME;
        });
        if (iter != mFlows.end()) return;
        
        auto playerSceneObjectOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        const auto& enemyName = *static_cast<strutils::StringId*>(secondBody->GetUserData());
        auto enemySceneObjectOpt = mScene.GetSceneObject(enemyName);
        
        if (playerSceneObjectOpt && enemySceneObjectOpt)
        {
            auto& playerSO = playerSceneObjectOpt->get();
            auto& enemySO = enemySceneObjectOpt->get();
            auto& playerSODef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(playerSO.mObjectFamilyTypeName)->get();
            auto enemySceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName)->get();
            
            if (mScene.GetSceneObject(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME))
            {
                mScene.RemoveAllSceneObjectsWithName(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
            }
            else
            {
                playerSO.mHealth -= enemySceneObjectTypeDef.mDamage;
                OnPlayerDamaged();
                CreateTextOnDamage(playerSO.mName, math::Box2dVec2ToGlmVec3(playerSO.mBody->GetWorldCenter()), enemySceneObjectTypeDef.mDamage);
            }
            
            mFlows.emplace_back([]()
            {
            }, game_constants::PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME);
            
            if (playerSO.mHealth <= 0)
            {
                scene_object_utils::ChangeSceneObjectState(playerSO, playerSODef, strutils::StringId("dying"));
                
                mFlows.emplace_back([=]()
                {
                    mScene.RemoveAllSceneObjectsWithName(game_constants::PLAYER_SCENE_OBJECT_NAME);
                }, playerSO.mAnimation->VGetDuration(), RepeatableFlow::RepeatPolicy::ONCE);
            }
        }
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_CATEGORY_BIT, physics_constants::ENEMY_BULLET_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        auto iter = std::find_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
        {
            return flow.GetName() == game_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME;
        });
        if (iter != mFlows.end()) return;
        
        auto playerSceneObjectOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        const auto& enemyBulletName = *static_cast<strutils::StringId*>(secondBody->GetUserData());
        auto enemyBulletSceneObjectOpt = mScene.GetSceneObject(enemyBulletName);
        
        if (playerSceneObjectOpt && enemyBulletSceneObjectOpt)
        {
            auto& playerSO = playerSceneObjectOpt->get();
            auto& enemyBulletSO = enemyBulletSceneObjectOpt->get();
            auto& playerSODef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(playerSO.mObjectFamilyTypeName)->get();
            
            auto enemyBulletSceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemyBulletSO.mObjectFamilyTypeName)->get();
            
            if (mScene.GetSceneObject(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME))
            {
                mScene.RemoveAllSceneObjectsWithName(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
            }
            else
            {
                playerSO.mHealth -= enemyBulletSceneObjectTypeDef.mDamage;
                OnPlayerDamaged();
                CreateTextOnDamage(playerSO.mName, math::Box2dVec2ToGlmVec3(playerSO.mBody->GetWorldCenter()), enemyBulletSceneObjectTypeDef.mDamage);
            }
            
            mFlows.emplace_back([]()
            {
            }, game_constants::PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME);
            
            if (playerSO.mHealth <= 0)
            {
                scene_object_utils::ChangeSceneObjectState(playerSO, playerSODef, strutils::StringId("dying"));
                
                mFlows.emplace_back([=]()
                {
                    mScene.RemoveAllSceneObjectsWithName(game_constants::PLAYER_SCENE_OBJECT_NAME);
                }, playerSO.mAnimation->VGetDuration(), RepeatableFlow::RepeatPolicy::ONCE);
            }
            
            // Erase bullet collision mask so that it doesn't also contribute to other
            // enemy damage until it is removed from b2World
            auto bulletFilter = secondBody->GetFixtureList()[0].GetFilterData();
            bulletFilter.maskBits = 0;
            secondBody->GetFixtureList()[0].SetFilterData(bulletFilter);
            
            RemoveWaveEnemy(enemyBulletName);
        }
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_BULLET_CATEGORY_BIT, physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        const auto& playerBulletName = *static_cast<strutils::StringId*>(firstBody->GetUserData());
        RemoveWaveEnemy(playerBulletName);
    });

    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_CATEGORY_BIT, physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        const auto& enemyName = *static_cast<strutils::StringId*>(firstBody->GetUserData());
        RemoveWaveEnemy(enemyName);
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_BULLET_CATEGORY_BIT, physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        const auto& enemyBulletName = *static_cast<strutils::StringId*>(firstBody->GetUserData());
        RemoveWaveEnemy(enemyBulletName);
    });
    
    mBox2dWorld.SetContactListener(&collisionListener);
    
    UpgradesLoader loader;
    GameSingletons::SetAvailableUpgrades(loader.LoadAllUpgrades());
    
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    mStateMachine.RegisterState<BossIntroGameState>();
    mStateMachine.RegisterState<FightingWaveGameState>();
    mStateMachine.RegisterState<WaveIntroGameState>();
    mStateMachine.RegisterState<PauseMenuGameState>();
    mStateMachine.RegisterState<UpgradeSelectionGameState>();

    LoadLevelInvariantObjects();
    mActiveLightNames.clear();
    mWaveEnemies.clear();
    mCurrentWaveNumber = 0;
    mStateMachine.InitStateMachine(WaveIntroGameState::STATE_NAME);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::OnAppStateChange(Uint32 event)
{
    static bool hasLeftForegroundOnce = false;
    
    switch (event)
    {
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND:
        {
#ifdef DEBUG
            hasLeftForegroundOnce = true;
#else
            if (mLastPostStateMachineUpdateDirective != PostStateUpdateDirective::BLOCK_UPDATE)
            {
                mStateMachine.PushState(PauseMenuGameState::STATE_NAME);
            }
#endif
        } break;
            
        case SDL_APP_WILLENTERFOREGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
        {
#ifdef DEBUG
            if (hasLeftForegroundOnce)
            {
                OpenDebugConsole();
            }
#endif
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
    
    mBox2dWorld.Step(physics_constants::WORLD_STEP * GameSingletons::GetGameSpeedMultiplier(), physics_constants::WORLD_VELOCITY_ITERATIONS, physics_constants::WORLD_POSITION_ITERATIONS);
    
    auto joystickSO = mScene.GetSceneObject(game_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(game_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    auto playerSO = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (joystickSO && joystickBoundsSO)
    {
        joystickSO->get().mInvisible = true;
        joystickBoundsSO->get().mInvisible = true;
    }
    
    for (auto& sceneObject: sceneObjects)
    {
        // Check if this scene object has a respective family object definition
        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject.mObjectFamilyTypeName);
        if (sceneObjectTypeDefOpt && !sceneObject.mCustomDrivenMovement)
        {
            // Update movement
            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
            switch (sceneObjectTypeDef.mMovementControllerPattern)
            {
                case MovementControllerPattern::CONSTANT_VELOCITY:
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
        
        if (sceneObject.mAnimation)
        {
            sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
        }
        
        for (auto& extraAnimation: sceneObject.mExtraCompoundingAnimations)
        {
            extraAnimation->VUpdate(dtMillis, sceneObject);
        }
    }
    
    const auto& bossName = mLevel.mWaves.at(mCurrentWaveNumber).mBossName;
    if (!bossName.isEmpty())
    {
        mBossAIController.UpdateBossAI(bossName, dtMillis);
    }
    
    mUpgradesLogicHandler.OnUpdate(dtMillis);
    UpdateBackground(dtMillis);
    UpdateHealthBars(dtMillis);
    UpdateFlows(dtMillis);
    UpdateCameras(dtMillis);
    UpdateLights(dtMillis);
    UpdateTextDamage(dtMillis);
}

///------------------------------------------------------------------------------------------------

std::string LevelUpdater::GetDescription() const
{
    return std::to_string(GetWaveEnemyCount());
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

void LevelUpdater::AddWaveEnemy(const strutils::StringId& enemyName)
{
    mWaveEnemies.insert(enemyName);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::RemoveWaveEnemy(const strutils::StringId& enemyName)
{
    mWaveEnemies.erase(enemyName);
    mScene.RemoveAllSceneObjectsWithName(enemyName);
    
    auto enemyBulletFlow = GetFlow(strutils::StringId(enemyName.GetString() + game_constants::ENEMY_PROJECTILE_FLOW_POSTFIX));
    if (enemyBulletFlow)
    {
        enemyBulletFlow->get().ForceFinish();
    }
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

#ifdef DEBUG
void LevelUpdater::OpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void LevelUpdater::OnBossPositioned()
{
    mStateMachine.PushState(BossIntroGameState::STATE_NAME);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateLevelWalls(const Camera& cam, const bool invisible)
{
    // L_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(-cam.GetCameraLenseWidth()/2.0f, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, cam.GetCameraLenseHeight() * 4);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::GLOBAL_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_constants::WALL_Z;
        so.mName = game_constants::WALL_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(so));
    }
    
    // R_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(cam.GetCameraLenseWidth()/2, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, cam.GetCameraLenseHeight() * 4);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::GLOBAL_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_constants::WALL_Z;
        so.mName = game_constants::WALL_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(so));
    }

    // PLAYER_ONLY_BOT_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, -cam.GetCameraLenseHeight()/2 + 1.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_constants::WALL_Z;
        so.mName = game_constants::WALL_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(so));
    }
    
    // ENEMY_ONLY_BOT_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, -cam.GetCameraLenseHeight()/2 - 7.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth() * 4, 2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mInvisible = invisible;
        so.mPosition.z = game_constants::WALL_Z;
        so.mName = game_constants::WALL_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(so));
    }
    
    // BULLET_TOP_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, cam.GetCameraLenseHeight()/2);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_constants::WALL_Z;
        so.mName = game_constants::WALL_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::LoadLevelInvariantObjects()
{
    mScene.GetLightRepository().AddLight(LightType::AMBIENT_LIGHT, strutils::StringId("AMBIENT_LIGHT"), game_constants::AMBIENT_LIGHT_COLOR, glm::vec3(0.0f), 0.0f);
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_constants::BACKGROUND_SCALE;
        bgSO.mPosition.z = game_constants::BACKGROUND_Z;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::TEXTURE_OFFSET_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = true;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // Player
    {
        auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::BETTER_PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::MIRROR_IMAGE_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE);
        
        auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject playerSO = scene_object_utils::CreateSceneObjectWithBody(playerObjectDef, game_constants::PLAYER_INITIAL_POS, mBox2dWorld, game_constants::PLAYER_SCENE_OBJECT_NAME);
        mScene.AddSceneObject(std::move(playerSO));
    }
    
    const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    assert(worldCamOpt);
    const auto& worldCam = worldCamOpt->get();
    
    CreateLevelWalls(worldCam, true);
    
    // Joystick
    {
        SceneObject joystickSO;
        joystickSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::JOYSTICK_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        joystickSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickSO.mScale = game_constants::JOYSTICK_SCALE;
        joystickSO.mName = game_constants::JOYSTICK_SCENE_OBJECT_NAME;
        joystickSO.mInvisible = true;
        mScene.AddSceneObject(std::move(joystickSO));
    }
    
    // Joystick Bounds
    {
        SceneObject joystickBoundsSO;
        joystickBoundsSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::JOYSTICK_BOUNDS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        joystickBoundsSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickBoundsSO.mScale = game_constants::JOYSTICK_BOUNDS_SCALE;
        joystickBoundsSO.mName = game_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME;
        joystickBoundsSO.mInvisible = true;
        mScene.AddSceneObject(std::move(joystickBoundsSO));
    }
    
    // Player Health Bar
    {
        SceneObject healthBarSo;
        healthBarSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mScale = game_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarSo.mName = game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(healthBarSo));
    }
    
    // Player Health Bar Frame
    {
        SceneObject healthBarFrameSo;
        healthBarFrameSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarFrameSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarFrameSo.mScale = game_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarFrameSo.mName = game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(healthBarFrameSo));
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    
    auto joystickSO = mScene.GetSceneObject(game_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(game_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    const auto& inputContext = GameSingletons::GetInputContext();
    
    switch (inputContext.mEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
                 joystickBoundsSO->get().mPosition.z = game_constants::JOYSTICK_Z;
                 
                 joystickSO->get().mPosition = joystickBoundsSO->get().mPosition;
                 joystickSO->get().mPosition.z = game_constants::JOYSTICK_BOUNDS_Z;

                 mAllowInputControl = true;
                 
                 mMovementRotationAllowed = true;
                 mPreviousMotionVec.x = mPreviousMotionVec.y = 0.0f;
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
                auto motionVec = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix()) - joystickBoundsSO->get().mPosition;

                glm::vec3 norm = glm::normalize(motionVec);
                if (glm::length(motionVec) > glm::length(norm))
                {
                    motionVec = norm;
                }
                
                joystickSO->get().mPosition = joystickBoundsSO->get().mPosition + motionVec;
                joystickSO->get().mPosition.z = game_constants::JOYSTICK_Z;
                
                motionVec.x *= sceneObjectTypeDef.mSpeed * dtMilis;
                motionVec.y *= sceneObjectTypeDef.mSpeed * dtMilis;
                
                if (motionVec.x > 0.0f && mPreviousMotionVec.x <= 0.0f && mMovementRotationAllowed)
                {
                    if (math::RandomFloat() < game_constants::PLAYER_MOVEMENT_ROLL_CHANCE)
                    {
                        sceneObject.mExtraCompoundingAnimations.clear();
                        sceneObject.mExtraCompoundingAnimations.push_back( std::make_unique<RotationAnimation>(sceneObject.mAnimation->VGetCurrentTextureResourceId(), sceneObject.mAnimation->VGetCurrentMeshResourceId(), sceneObject.mAnimation->VGetCurrentShaderResourceId(), sceneObject.mAnimation->VGetScale(), RotationAnimation::RotationMode::ROTATE_TO_TARGET_ONCE, RotationAnimation::RotationAxis::Y, game_constants::PLAYER_MOVEMENT_ROLL_ANGLE, game_constants::PLAYER_MOVEMENT_ROLL_SPEED, true));
                    }
                    
                    mMovementRotationAllowed = false;
                }
                else if (motionVec.x < 0.0f && mPreviousMotionVec.x >= 0.0f && mMovementRotationAllowed)
                {
                    if (math::RandomFloat() < game_constants::PLAYER_MOVEMENT_ROLL_CHANCE)
                    {
                        sceneObject.mExtraCompoundingAnimations.clear();
                        sceneObject.mExtraCompoundingAnimations.push_back( std::make_unique<RotationAnimation>(sceneObject.mAnimation->VGetCurrentTextureResourceId(), sceneObject.mAnimation->VGetCurrentMeshResourceId(), sceneObject.mAnimation->VGetCurrentShaderResourceId(), sceneObject.mAnimation->VGetScale(), RotationAnimation::RotationMode::ROTATE_TO_TARGET_ONCE, RotationAnimation::RotationAxis::Y, -game_constants::PLAYER_MOVEMENT_ROLL_ANGLE, game_constants::PLAYER_MOVEMENT_ROLL_SPEED, true));
                    }
                    
                    mMovementRotationAllowed = false;
                }
                
                sceneObject.mBody->SetLinearVelocity(b2Vec2(motionVec.x, motionVec.y));
                
                mPreviousMotionVec = motionVec;
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

void LevelUpdater::UpdateBackground(const float dtMillis)
{
    static float msAccum = 0.0f;
    msAccum += dtMillis * game_constants::BACKGROUND_SPEED;
    msAccum = std::fmod(msAccum, 1.0f);
    
    auto bgSO = mScene.GetSceneObject(game_constants::BACKGROUND_SCENE_OBJECT_NAME);
    if (bgSO)
    {
       bgSO->get().mShaderFloatUniformValues[game_constants::GENERIC_TEXTURE_OFFSET_UNIFORM_NAME] = -msAccum;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateHealthBars(const float dtMillis)
{
    // Player health bar
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerHealthBarFrameSoOpt = mScene.GetSceneObject(game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto playerHealthBarSoOpt = mScene.GetSceneObject(game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME);
    
    if (playerSoOpt && playerHealthBarSoOpt && playerHealthBarFrameSoOpt)
    {
        auto& playerSo = playerSoOpt->get();
        auto& healthBarFrameSo = playerHealthBarFrameSoOpt->get();
        auto& healthBarSo = playerHealthBarSoOpt->get();
        
        auto& playerObjectDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(playerSo.mObjectFamilyTypeName)->get();
        
        healthBarSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mPosition.z = game_constants::PLAYER_HEALTH_BAR_Z;
        
        healthBarFrameSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        
        float healthPerc = playerSo.mHealth/static_cast<float>(playerObjectDef.mHealth);
        
        healthBarSo.mScale.x = game_constants::PLAYER_HEALTH_BAR_SCALE.x * mPlayerAnimatedHealthBarPerc;
        healthBarSo.mPosition.x -= (1.0f - mPlayerAnimatedHealthBarPerc)/game_constants::HEALTH_BAR_POSITION_DIVISOR_MAGIC * game_constants::PLAYER_HEALTH_BAR_SCALE.x;
        
        if (healthPerc < mPlayerAnimatedHealthBarPerc)
        {
            mPlayerAnimatedHealthBarPerc -= game_constants::HEALTH_LOST_SPEED * dtMillis;
            if (mPlayerAnimatedHealthBarPerc <= healthPerc)
            {
                mPlayerAnimatedHealthBarPerc = healthPerc;
            }
        }
        else
        {
            mPlayerAnimatedHealthBarPerc += game_constants::HEALTH_LOST_SPEED * dtMillis;
            if (mPlayerAnimatedHealthBarPerc >= healthPerc)
            {
                mPlayerAnimatedHealthBarPerc = healthPerc;
            }
        }
    }
    else
    {
        playerHealthBarFrameSoOpt->get().mInvisible = true;
        playerHealthBarSoOpt->get().mInvisible = true;
    }
    
    // Boss Health bar
    auto bossHealthBarFrameSoOpt = mScene.GetSceneObject(game_constants::BOSS_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto bossHealthBarSoOpt = mScene.GetSceneObject(game_constants::BOSS_HEALTH_BAR_SCENE_OBJECT_NAME);
    
    if (bossHealthBarFrameSoOpt && bossHealthBarSoOpt)
    {
        auto& healthBarFrameSo = bossHealthBarFrameSoOpt->get();
        auto& healthBarSo = bossHealthBarSoOpt->get();
            
        if (!healthBarSo.mInvisible && !healthBarFrameSo.mInvisible)
        {
            healthBarSo.mPosition = game_constants::BOSS_HEALTH_BAR_POSITION;
            healthBarSo.mPosition.z = game_constants::BOSS_HEALTH_BAR_Z;
            
            healthBarFrameSo.mPosition = game_constants::BOSS_HEALTH_BAR_POSITION;
            
            double healthPerc = GameSingletons::GetBossCurrentHealth()/GameSingletons::GetBossMaxHealth();
            
            healthBarSo.mScale.x = game_constants::BOSS_HEALTH_BAR_SCALE.x * mBossAnimatedHealthBarPerc;
            healthBarSo.mPosition.x -= (1.0f - mBossAnimatedHealthBarPerc)/game_constants::HEALTH_BAR_POSITION_DIVISOR_MAGIC * game_constants::BOSS_HEALTH_BAR_SCALE.x;
            
            if (healthPerc < mBossAnimatedHealthBarPerc)
            {
                mBossAnimatedHealthBarPerc -= game_constants::HEALTH_LOST_SPEED * dtMillis;
                if (mBossAnimatedHealthBarPerc <= healthPerc)
                {
                    mBossAnimatedHealthBarPerc = healthPerc;
                }
            }
            else
            {
                mBossAnimatedHealthBarPerc += game_constants::HEALTH_LOST_SPEED * dtMillis;
                if (mBossAnimatedHealthBarPerc >= healthPerc)
                {
                    mBossAnimatedHealthBarPerc = healthPerc;
                }
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateFlows(const float dtMillis)
{
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateCameras(const float dtMillis)
{
    const auto& guiCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    
    if (guiCamOpt) guiCamOpt->get().Update(dtMillis);
    if (worldCamOpt) worldCamOpt->get().Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateLights(const float dtMillis)
{
    auto& lightRepository = mScene.GetLightRepository();
    
    auto lightIter = mActiveLightNames.begin();
    while (lightIter != mActiveLightNames.end())
    {
        auto lightName = *lightIter;
        auto lightIndex = lightRepository.GetLightIndex(lightName);
        auto lightPower = lightRepository.GetLightPower(lightIndex);
        if (lightPower < 0.0f)
        {
            lightRepository.RemoveLight(lightName);
            lightIter = mActiveLightNames.erase(lightIter);
        }
        else
        {
            lightPower -= dtMillis * game_constants::EXPLOSION_LIGHT_FADE_SPEED;
            lightRepository.SetLightPower(lightIndex, lightPower);
            lightIter++;
        }
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateTextDamage(const float dtMillis)
{
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> sceneObjectEntriesToRemove;
    
    for (auto& damagedSceneObjectToTextEntry: mDamageTextSceneObjects)
    {
        auto sceneObjectOpt = mScene.GetSceneObject(damagedSceneObjectToTextEntry.second);
        if (sceneObjectOpt)
        {
            auto& sceneObject = sceneObjectOpt->get();
            
            if (mDamageTextSceneObjectsFreezeTimers[damagedSceneObjectToTextEntry.first] > 0.0f)
            {
                mDamageTextSceneObjectsFreezeTimers[damagedSceneObjectToTextEntry.first] -= dtMillis;
                sceneObject.mAnimation->VPause();
            }
            else
            {
                sceneObject.mAnimation->VResume();
                auto& sceneObjectAlpha = sceneObject.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].w;
                
                sceneObjectAlpha -= game_constants::TEXT_DAMAGE_ALPHA_SPEED * dtMillis;
                if (sceneObjectAlpha <= 0.0f)
                {
                    sceneObjectAlpha = 0.0f;
                    mScene.RemoveAllSceneObjectsWithName(damagedSceneObjectToTextEntry.second);
                    sceneObjectEntriesToRemove.insert(damagedSceneObjectToTextEntry.first);
                }
            }
        }
    }
    
    for (auto& sceneObjectEntryToRemove: sceneObjectEntriesToRemove)
    {
        mDamageTextSceneObjects.erase(sceneObjectEntryToRemove);
        mDamageTextSceneObjectsFreezeTimers.erase(sceneObjectEntryToRemove);
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateTextOnDamage(const strutils::StringId& damagedSceneObjectName, const glm::vec3& textOriginPos, const int damage)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    auto existingDamageTextForSceneObjectIter = mDamageTextSceneObjects.find(damagedSceneObjectName);
    if (existingDamageTextForSceneObjectIter != mDamageTextSceneObjects.cend())
    {
        auto existingDamageTextSceneObjectOpt = mScene.GetSceneObject(existingDamageTextForSceneObjectIter->second);
        if (existingDamageTextSceneObjectOpt)
        {
            existingDamageTextSceneObjectOpt->get().mText = std::to_string(std::stoi(existingDamageTextSceneObjectOpt->get().mText) + damage);
            
            mDamageTextSceneObjectsFreezeTimers[damagedSceneObjectName] = 300.0f;
        }
    }
    else
    {
        bool enemyDamaged = damagedSceneObjectName != game_constants::PLAYER_SCENE_OBJECT_NAME;
        glm::vec3 firstControlPoint = glm::vec3(textOriginPos.x, textOriginPos.y, game_constants::DAMAGE_TEXT_Z);
        glm::vec3 secondControlPoint = glm::vec3(textOriginPos.x, textOriginPos.y + (enemyDamaged ? game_constants::TEXT_DAMAGE_ENEMY_Y_MAX_DISPLACEMENT : game_constants::TEXT_DAMAGE_PLAYER_Y_MAX_DISPLACEMENT), game_constants::DAMAGE_TEXT_Z);
        
        math::BezierCurve curve({ firstControlPoint, secondControlPoint });
        
        SceneObject damageTextSO;
        damageTextSO.mPosition = textOriginPos;
        damageTextSO.mPosition.z = game_constants::DAMAGE_TEXT_Z;
        damageTextSO.mScale = game_constants::TEXT_DAMAGE_SCALE;
        damageTextSO.mAnimation = std::make_unique<BezierCurvePathAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME), glm::vec3(1.0f), curve, game_constants::TEXT_DAMAGE_CURVE_TRAVERSAL_SPEED, false);
        damageTextSO.mFontName = game_constants::DEFAULT_FONT_NAME;
        damageTextSO.mSceneObjectType = SceneObjectType::GUIObject;
        damageTextSO.mName = strutils::StringId(std::to_string(SDL_GetTicks64()));
        damageTextSO.mText = std::to_string(damage);
        damageTextSO.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = enemyDamaged ? glm::vec4(1.0f, 1.0f, 1.0f, 0.8f) : glm::vec4(1.0f, 0.0f, 0.0f, 0.8f);
        
        mDamageTextSceneObjects[damagedSceneObjectName] = damageTextSO.mName;
        mDamageTextSceneObjectsFreezeTimers[damagedSceneObjectName] = 300.0f;
        mScene.AddSceneObject(std::move(damageTextSO));
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::OnPlayerDamaged()
{
    objectiveC_utils::Vibrate();
    
    const auto& guiCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    
    if (guiCamOpt) guiCamOpt->get().Shake();
    if (worldCamOpt) worldCamOpt->get().Shake();
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::OnBlockedUpdate()
{
    mAllowInputControl = false;
    auto joystickSoOpt = mScene.GetSceneObject(game_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSOopt = mScene.GetSceneObject(game_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    
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
        auto bulletPos = position;
        bulletPos.z = game_constants::BULLET_Z;
        mScene.AddSceneObject(scene_object_utils::CreateSceneObjectWithBody(bulletDef, position, mBox2dWorld));
    }
}

///------------------------------------------------------------------------------------------------
