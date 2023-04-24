///------------------------------------------------------------------------------------------------
///  LevelUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "BlueprintFlows.h"
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
#include "states/ClearedLevelAnimationGameState.h"
#include "states/DebugConsoleGameState.h"
#include "states/FightingWaveGameState.h"
#include "states/WaveIntroGameState.h"
#include "states/PauseMenuGameState.h"

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

static const std::string DROPPED_CRYSTAL_NAME_PREFIX = "DROPPED_CRYSTAL_";

static const glm::vec4 ENEMY_TEXT_DAMAGE_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
static const glm::vec4 PLAYER_TEXT_DAMAGE_COLOR = glm::vec4(1.0f, 0.3f, 0.3f, 0.8f);

static const glm::vec3 TEXT_DAMAGE_SCALE = glm::vec3(0.006f, 0.006f, 1.0f);

static const glm::vec3 JOYSTICK_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);
static const glm::vec3 JOYSTICK_BOUNDS_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);

static const float JOYSTICK_Z = 1.0f;
static const float JOYSTICK_BOUNDS_Z = 2.0f;

static const float PLAYER_MOVEMENT_ROLL_CHANCE = 0.333f;
static const float PLAYER_MOVEMENT_ROLL_SPEED = 0.008f;
static const float PLAYER_MOVEMENT_ROLL_ANGLE = 180.0f;

static const float EXPLOSION_LIGHT_POWER = 1.0f;
static const float EXPLOSION_LIGHT_FADE_SPEED = 1.0f/400.0f;

static const float TEXT_DAMAGE_Y_OFFSET = 1.5f;
static const float TEXT_DAMAGE_X_OFFSET = -0.2f;
static const float TEXT_DAMAGE_MOVEMENT_SPEED = 0.002f;
static const float TEXT_DAMAGE_FREEZE_MILLIS = 300.0f;
static const float TEXT_DAMAGE_Z = 2.0f;

static const float DROPPED_CRYSTAL_SPEED = 0.0009f;
static const float DROPPED_CRYSTAL_DISTANCE_FACTOR = 24.0f;
static const float DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG = 0.5f;
static const float DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG = 2.0f;
static const float COLLECTED_CRYSTAL_PULSING_SPEED = 0.02f;
static const float COLLECTED_CRYSTAL_PULSING_FACTOR = 0.01f;

static const float SHAKE_ENTITY_HEALTH_RATIO_THRESHOLD = 0.2f;
static const float SHAKE_ENTITY_RANDOM_MAG = 0.03f;

static const float MIRROR_IMAGE_BULLET_DAMAGE_MULTIPLIER = 0.3f;

///------------------------------------------------------------------------------------------------

LevelUpdater::LevelUpdater(Scene& scene, b2World& box2dWorld, LevelDefinition&& levelDef)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mUpgradesLogicHandler(scene)
    , mStateMachine(&scene, this, &mUpgradesLogicHandler, &mBox2dWorld)
    , mBossAIController(scene, *this, mStateMachine, mBox2dWorld)
    , mCurrentWaveNumber(0)
    , mBossAnimatedHealthBarPerc(0.0f)
    , mAllowInputControl(false)
    , mMovementRotationAllowed(false)
    , mBossPositioned(false)
{
    mLevel = levelDef;
    
    blueprint_flows::CreatePlayerBulletFlow(mFlows, mScene, mBox2dWorld);
    
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
                auto bulletDamage = GameSingletons::GetPlayerAttackStat();
                if (bulletSceneObjectTypeDef.mName == game_constants::MIRROR_IMAGE_BULLET_TYPE)
                {
                    bulletDamage *= MIRROR_IMAGE_BULLET_DAMAGE_MULTIPLIER;
                }
                
                if (scene_object_utils::IsSceneObjectBossPart(enemySO))
                {
                    GameSingletons::SetBossCurrentHealth(math::Max(0.0f, GameSingletons::GetBossCurrentHealth() - bulletDamage));
                }
                else
                {
                    enemySO.mHealth -= bulletDamage;
                    CreateTextOnDamage(enemySO.mName, math::Box2dVec2ToGlmVec3(enemySO.mBody->GetWorldCenter()), bulletDamage);
                }
            }
            
            if (enemySO.mHealth <= 0)
            {
                scene_object_utils::ChangeSceneObjectState(enemySO, enemySceneObjectTypeDef, game_constants::DYING_SCENE_OBJECT_STATE);
                
                const auto deathPosition = enemySO.mPosition;
                const auto crystalYield = enemySceneObjectTypeDef.mCrystalYield;
                
                DropCrystals(deathPosition, enemySO.mAnimation->VGetDurationMillis(), crystalYield);
                
                mFlows.emplace_back([=]()
                {
                    RemoveWaveEnemy(enemyName);
                }, enemySO.mAnimation->VGetDurationMillis(), RepeatableFlow::RepeatPolicy::ONCE);
                
                mActiveLightNames.insert(enemyName);
                mScene.GetLightRepository().AddLight(LightType::POINT_LIGHT, enemyName, game_constants::POINT_LIGHT_COLOR, enemySO.mPosition, EXPLOSION_LIGHT_POWER);
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
            auto enemySceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName)->get();
            
            if (!playerSO.mInvulnerable)
            {
                // Remove player shield/damage player flow
                auto incomingDamage = enemySceneObjectTypeDef.mDamage - GameSingletons::GetPlayerShieldHealth();
                if (GameSingletons::GetPlayerShieldHealth() > 0.0f)
                {
                    GameSingletons::SetPlayerShieldHealth(GameSingletons::GetPlayerShieldHealth() - enemySceneObjectTypeDef.mDamage);
                    
                    if (GameSingletons::GetPlayerShieldHealth() <= 0.0f)
                    {
                        auto playerShieldOpt = mScene.GetSceneObject(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
                        if (playerShieldOpt)
                        {
                            playerShieldOpt->get().mAnimation->VResume();
                        }
                    }
                }
                
                if (incomingDamage > 0.0f)
                {
                    GameSingletons::SetPlayerCurrentHealth(GameSingletons::GetPlayerCurrentHealth() - incomingDamage);
                    OnPlayerDamaged();
                    CreateTextOnDamage(playerSO.mName, math::Box2dVec2ToGlmVec3(playerSO.mBody->GetWorldCenter()), incomingDamage);
                }
                
                // Kamikaze everything that isn't a boss part
                if (!scene_object_utils::IsSceneObjectBossPart(enemySO))
                {
                    scene_object_utils::ChangeSceneObjectState(enemySO, enemySceneObjectTypeDef, game_constants::DYING_SCENE_OBJECT_STATE);
                    
                    mFlows.emplace_back([=]()
                                        {
                        RemoveWaveEnemy(enemyName);
                    }, enemySO.mAnimation->VGetDurationMillis(), RepeatableFlow::RepeatPolicy::ONCE);
                    
                    mActiveLightNames.insert(enemyName);
                    mScene.GetLightRepository().AddLight(LightType::POINT_LIGHT, enemyName, game_constants::POINT_LIGHT_COLOR, enemySO.mPosition, EXPLOSION_LIGHT_POWER);
                }
                
                // Enable invincibility flow
                mFlows.emplace_back([]()
                                    {
                }, game_constants::PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME);
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
            
            if (!playerSO.mInvulnerable)
            {
                auto enemyBulletSceneObjectTypeDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemyBulletSO.mObjectFamilyTypeName)->get();
                
                // Remove player shield/damage player flow
                auto incomingDamage = enemyBulletSceneObjectTypeDef.mDamage - GameSingletons::GetPlayerShieldHealth();
                if (GameSingletons::GetPlayerShieldHealth() > 0.0f)
                {
                    GameSingletons::SetPlayerShieldHealth(GameSingletons::GetPlayerShieldHealth() - enemyBulletSceneObjectTypeDef.mDamage);
                    
                    if (GameSingletons::GetPlayerShieldHealth() <= 0.0f)
                    {
                        auto playerShieldOpt = mScene.GetSceneObject(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
                        if (playerShieldOpt)
                        {
                            playerShieldOpt->get().mAnimation->VResume();
                        }
                    }
                }
                
                if (incomingDamage > 0.0f)
                {
                    GameSingletons::SetPlayerCurrentHealth(GameSingletons::GetPlayerCurrentHealth() - incomingDamage);
                    OnPlayerDamaged();
                    CreateTextOnDamage(playerSO.mName, math::Box2dVec2ToGlmVec3(playerSO.mBody->GetWorldCenter()), incomingDamage);
                }
                
                mFlows.emplace_back([]()
                {
                }, game_constants::PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, game_constants::PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME);
                
                // Erase bullet collision mask so that it doesn't also contribute to other
                // enemy damage until it is removed from b2World
                auto bulletFilter = secondBody->GetFixtureList()[0].GetFilterData();
                bulletFilter.maskBits = 0;
                secondBody->GetFixtureList()[0].SetFilterData(bulletFilter);
                
                RemoveWaveEnemy(enemyBulletName);
            }
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
    
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    mStateMachine.RegisterState<BossIntroGameState>();
    mStateMachine.RegisterState<ClearedLevelAnimationGameState>();
    mStateMachine.RegisterState<FightingWaveGameState>();
    mStateMachine.RegisterState<WaveIntroGameState>();
    mStateMachine.RegisterState<PauseMenuGameState>();

    LoadLevelInvariantObjects();
    mActiveLightNames.clear();
    mWaveEnemies.clear();
    mCurrentWaveNumber = 0;
    mStateMachine.InitStateMachine(WaveIntroGameState::STATE_NAME);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::VOnAppStateChange(Uint32 event)
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
                VOpenDebugConsole();
            }
#endif
        } break;
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective LevelUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    // State machines empties out once the level is finished
    if (mStateMachine.IsEmpty())
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    // A BLOCK_UPDATE directive from the FSM signals a e.g. popup, the existence of which
    // makes it so that we should skip the rest of the world/SOs/Elements in the scene from updating below
    mLastPostStateMachineUpdateDirective = mStateMachine.Update(dtMillis);
    if (mLastPostStateMachineUpdateDirective == PostStateUpdateDirective::BLOCK_UPDATE)
    {
        OnBlockedUpdate();
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    // Physics update
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
        
        if (sceneObject.mAnimation && !sceneObject.mAnimation->VIsPaused())
        {
            sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
        }
        
        for (auto& extraAnimation: sceneObject.mExtraCompoundingAnimations)
        {
            if (!extraAnimation->VIsPaused())
            {
                extraAnimation->VUpdate(dtMillis, sceneObject);
            }
        }
    }
    
    if (!LevelFinished())
    {
        const auto& bossName = mLevel.mWaves.at(mCurrentWaveNumber).mBossName;
        if (!bossName.isEmpty())
        {
            mBossAIController.UpdateBossAI(bossName, dtMillis);
        }
    }
    
    mUpgradesLogicHandler.Update(dtMillis);
    ApplyShakeToNearlyDeadEntities(sceneObjects);
    UpdateBackground(dtMillis);
    UpdateBossHealthBar(dtMillis);
    UpdateFlows(dtMillis);
    UpdateCameras(dtMillis);
    UpdateLights(dtMillis);
    UpdateTextDamage(dtMillis);
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

std::string LevelUpdater::VGetDescription() const
{
    return std::to_string(GetWaveEnemyCount());
}

///------------------------------------------------------------------------------------------------

strutils::StringId LevelUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
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

void LevelUpdater::DropCrystals(const glm::vec3& deathPosition, const float enemyDeathAnimationMillis, float crystalYieldValue)
{
    // A value of crystalYield <= 1.0f will be random depending on the value,
    // however a value of > 1.0f such as 1.5f will guarantee 1 plus 1 half of the time and so on.
    auto droppedCrystalCounter = 0;
    
    while (crystalYieldValue > 0.0f)
    {
        if ((crystalYieldValue <= 1.0f && math::RandomFloat() <= crystalYieldValue) || crystalYieldValue > 1.0f)
        {
            mFlows.emplace_back([this, deathPosition, droppedCrystalCounter]()
            {
                auto& resService = resources::ResourceLoadingService::GetInstance();
                
                SceneObject crystalSo;
                
                glm::vec3 firstControlPoint(deathPosition + glm::vec3(math::RandomFloat(-DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG), math::RandomFloat(-DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG), 0.0f));
                glm::vec3 thirdControlPoint(game_constants::GUI_CRYSTAL_POSITION);
                glm::vec3 secondControlPoint((thirdControlPoint + firstControlPoint) * 0.5f + glm::vec3(math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), 0.0f));
                
                firstControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
                secondControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
                thirdControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
                
                float speedNoise = math::RandomFloat(-DROPPED_CRYSTAL_SPEED/5, DROPPED_CRYSTAL_SPEED/5);
                float speedMultiplier = DROPPED_CRYSTAL_DISTANCE_FACTOR/glm::distance(firstControlPoint, game_constants::GUI_CRYSTAL_POSITION);
                
                const strutils::StringId droppedCrystalName = strutils::StringId(DROPPED_CRYSTAL_NAME_PREFIX + std::to_string(SDL_GetPerformanceCounter()));
                
                crystalSo.mAnimation = std::make_unique<BezierCurvePathAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), math::BezierCurve({firstControlPoint, secondControlPoint, thirdControlPoint}), (DROPPED_CRYSTAL_SPEED + speedNoise) * speedMultiplier, false);
                
                crystalSo.mAnimation->SetCompletionCallback([droppedCrystalName, this]()
                {
                    auto crystalHolderSoOpt = mScene.GetSceneObject(game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME);
                    if (crystalHolderSoOpt)
                    {
                        auto& crystalHolderSo = crystalHolderSoOpt->get();
                        crystalHolderSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
                        
                        crystalHolderSo.mExtraCompoundingAnimations.clear();
                        crystalHolderSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(crystalHolderSo.mAnimation->VGetCurrentTextureResourceId(), crystalHolderSo.mAnimation->VGetCurrentMeshResourceId(), crystalHolderSo.mAnimation->VGetCurrentShaderResourceId(), game_constants::GUI_CRYSTAL_SCALE, PulsingAnimation::PulsingMode::OUTER_PULSE_ONCE, 0.0f, COLLECTED_CRYSTAL_PULSING_SPEED, COLLECTED_CRYSTAL_PULSING_FACTOR, false));
                    }
                    
                    mScene.RemoveAllSceneObjectsWithName(droppedCrystalName);
                    GameSingletons::SetCrystalCount(GameSingletons::GetCrystalCount() + 1);
                });
                
                crystalSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false));
                
                crystalSo.mSceneObjectType = SceneObjectType::GUIObject;
                crystalSo.mPosition = firstControlPoint;
                crystalSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
                crystalSo.mName = droppedCrystalName;
                mScene.AddSceneObject(std::move(crystalSo));
            }, droppedCrystalCounter * game_constants::DROPPED_CRYSTALS_CREATION_STAGGER_MILLIS + enemyDeathAnimationMillis, RepeatableFlow::RepeatPolicy::ONCE);
            
            droppedCrystalCounter++;
        }
        
        crystalYieldValue -= 1.0f;
    }
}

///------------------------------------------------------------------------------------------------

const LevelDefinition& LevelUpdater::GetCurrentLevelDefinition() const
{
    return mLevel;
}

///------------------------------------------------------------------------------------------------

bool LevelUpdater::LevelFinished() const
{
    return mCurrentWaveNumber >= mLevel.mWaves.size();
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

const std::unordered_set<strutils::StringId, strutils::StringIdHasher>& LevelUpdater::GetWaveEnemyNames() const
{
    return mWaveEnemies;
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void LevelUpdater::VOpenDebugConsole()
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
    mBossPositioned = true;
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
    mScene.GetLightRepository().AddLight(LightType::AMBIENT_LIGHT, game_constants::AMBIENT_LIGHT_NAME, game_constants::AMBIENT_LIGHT_COLOR, glm::vec3(0.0f), 0.0f);
    
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
        typeDefRepo.LoadObjectTypeDefinition(game_constants::MIRROR_IMAGE_BULLET_TYPE);
        
        auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject playerSO = scene_object_utils::CreateSceneObjectWithBody(playerObjectDef, game_constants::PLAYER_INITIAL_POS, mBox2dWorld, game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        mScene.AddSceneObject(std::move(playerSO));
        
        for (const auto& upgradeEntry: GameSingletons::GetEquippedUpgrades())
        {
            mUpgradesLogicHandler.InitializeEquippedUpgrade(upgradeEntry.mUpgradeNameId);
        }
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
        joystickSO.mScale = JOYSTICK_SCALE;
        joystickSO.mName = game_constants::JOYSTICK_SCENE_OBJECT_NAME;
        joystickSO.mInvisible = true;
        mScene.AddSceneObject(std::move(joystickSO));
    }
    
    // Joystick Bounds
    {
        SceneObject joystickBoundsSO;
        joystickBoundsSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::JOYSTICK_BOUNDS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        joystickBoundsSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickBoundsSO.mScale = JOYSTICK_BOUNDS_SCALE;
        joystickBoundsSO.mName = game_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME;
        joystickBoundsSO.mInvisible = true;
        mScene.AddSceneObject(std::move(joystickBoundsSO));
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMillis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    
    auto joystickSO = mScene.GetSceneObject(game_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(game_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    const auto& inputContext = GameSingletons::GetInputContext();
    
    if (GameSingletons::GetPlayerCurrentHealth() <= 0.0f)
    {
        if (joystickSO && joystickBoundsSO && mAllowInputControl)
        {
            joystickSO->get().mInvisible = true;
            joystickBoundsSO->get().mInvisible = true;
            sceneObject.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        }
        
        return;
    }
    
    switch (inputContext.mEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
                 joystickBoundsSO->get().mPosition.z = JOYSTICK_Z;
                 
                 joystickSO->get().mPosition = joystickBoundsSO->get().mPosition;
                 joystickSO->get().mPosition.z = JOYSTICK_BOUNDS_Z;

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
                joystickSO->get().mPosition.z = JOYSTICK_Z;
                
                motionVec.x *= game_constants::BASE_PLAYER_SPEED * GameSingletons::GetPlayerMovementSpeedStat() * dtMillis;
                motionVec.y *= game_constants::BASE_PLAYER_SPEED * GameSingletons::GetPlayerMovementSpeedStat() * dtMillis;
                
                if (motionVec.x > 0.0f && mPreviousMotionVec.x <= 0.0f && mMovementRotationAllowed)
                {
                    if (math::RandomFloat() < PLAYER_MOVEMENT_ROLL_CHANCE)
                    {
                        sceneObject.mExtraCompoundingAnimations.clear();
                        sceneObject.mExtraCompoundingAnimations.push_back( std::make_unique<RotationAnimation>(sceneObject.mAnimation->VGetCurrentTextureResourceId(), sceneObject.mAnimation->VGetCurrentMeshResourceId(), sceneObject.mAnimation->VGetCurrentShaderResourceId(), sceneObject.mAnimation->VGetScale(), RotationAnimation::RotationMode::ROTATE_TO_TARGET_ONCE, RotationAnimation::RotationAxis::Y, PLAYER_MOVEMENT_ROLL_ANGLE, PLAYER_MOVEMENT_ROLL_SPEED, true));
                    }
                    
                    mMovementRotationAllowed = false;
                }
                else if (motionVec.x < 0.0f && mPreviousMotionVec.x >= 0.0f && mMovementRotationAllowed)
                {
                    if (math::RandomFloat() < PLAYER_MOVEMENT_ROLL_CHANCE)
                    {
                        sceneObject.mExtraCompoundingAnimations.clear();
                        sceneObject.mExtraCompoundingAnimations.push_back( std::make_unique<RotationAnimation>(sceneObject.mAnimation->VGetCurrentTextureResourceId(), sceneObject.mAnimation->VGetCurrentMeshResourceId(), sceneObject.mAnimation->VGetCurrentShaderResourceId(), sceneObject.mAnimation->VGetScale(), RotationAnimation::RotationMode::ROTATE_TO_TARGET_ONCE, RotationAnimation::RotationAxis::Y, -PLAYER_MOVEMENT_ROLL_ANGLE, PLAYER_MOVEMENT_ROLL_SPEED, true));
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

void LevelUpdater::UpdateBossHealthBar(const float dtMillis)
{
    // Boss Health bar
    auto bossHealthBarFrameSoOpt = mScene.GetSceneObject(game_constants::BOSS_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto bossHealthBarSoOpt = mScene.GetSceneObject(game_constants::BOSS_HEALTH_BAR_SCENE_OBJECT_NAME);
    auto bossHealthBarTextSoOpt = mScene.GetSceneObject(game_constants::BOSS_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME);
    
    if (bossHealthBarFrameSoOpt && bossHealthBarSoOpt && bossHealthBarTextSoOpt)
    {
        auto& healthBarFrameSo = bossHealthBarFrameSoOpt->get();
        auto& healthBarSo = bossHealthBarSoOpt->get();
        auto& healthBarTextSo = bossHealthBarTextSoOpt->get();
        
        if (GameSingletons::GetBossCurrentHealth()/GameSingletons::GetBossMaxHealth() <= 0.0f)
        {
            healthBarFrameSo.mInvisible = true;
            healthBarSo.mInvisible = true;
            healthBarTextSo.mInvisible = true;
        }
            
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

            healthBarTextSo.mText = std::to_string(static_cast<int>(mBossAnimatedHealthBarPerc * GameSingletons::GetBossMaxHealth()));
            glm::vec2 botLeftRect, topRightRect;
            scene_object_utils::GetSceneObjectBoundingRect(healthBarTextSo, botLeftRect, topRightRect);
            healthBarTextSo.mPosition = game_constants::BOSS_HEALTH_BAR_POSITION + game_constants::HEALTH_BAR_TEXT_OFFSET;
            healthBarTextSo.mPosition.x -= (math::Abs(botLeftRect.x - topRightRect.x)/2.0f);
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
            lightPower -= dtMillis * EXPLOSION_LIGHT_FADE_SPEED;
            lightRepository.SetLightPower(lightIndex, lightPower);
            lightIter++;
        }
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateTextDamage(const float dtMillis)
{
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> sceneObjectEntriesToRemove;
    
    for (auto& damagedSceneObjectToTextEntry: mDamagedSceneObjectNameToTextSceneObject)
    {
        auto sceneObjectOpt = mScene.GetSceneObject(damagedSceneObjectToTextEntry.second);
        if (sceneObjectOpt)
        {
            auto& sceneObject = sceneObjectOpt->get();
            
            if (mDamagedSceneObjectNameToTextSceneObjectFreezeTimer[damagedSceneObjectToTextEntry.first] > 0.0f)
            {
                mDamagedSceneObjectNameToTextSceneObjectFreezeTimer[damagedSceneObjectToTextEntry.first] -= dtMillis;
                sceneObject.mAnimation->VPause();
                
                auto damagedSceneObjectOpt = mScene.GetSceneObject(damagedSceneObjectToTextEntry.first);
                if (damagedSceneObjectOpt && damagedSceneObjectOpt->get().mHealth > 0)
                {
                    sceneObject.mPosition = math::Box2dVec2ToGlmVec3(damagedSceneObjectOpt->get().mBody->GetWorldCenter());
                    sceneObject.mPosition.x += TEXT_DAMAGE_X_OFFSET;
                    sceneObject.mPosition.y += TEXT_DAMAGE_Y_OFFSET;
                    sceneObject.mPosition.z = TEXT_DAMAGE_Z;
                }
            }
            else
            {
                sceneObject.mAnimation->VResume();
                auto& sceneObjectAlpha = sceneObject.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].w;
                
                sceneObjectAlpha -= game_constants::TEXT_FADE_IN_ALPHA_SPEED * dtMillis;
                if (sceneObjectAlpha <= 0.0f)
                {
                    sceneObjectAlpha = 0.0f;
                    mScene.RemoveAllSceneObjectsWithName(damagedSceneObjectToTextEntry.second);
                    sceneObjectEntriesToRemove.insert(damagedSceneObjectToTextEntry.first);
                }
                
                sceneObject.mPosition.y += TEXT_DAMAGE_MOVEMENT_SPEED * dtMillis;
            }
        }
    }
    
    for (auto& sceneObjectEntryToRemove: sceneObjectEntriesToRemove)
    {
        mDamagedSceneObjectNameToTextSceneObject.erase(sceneObjectEntryToRemove);
        mDamagedSceneObjectNameToTextSceneObjectFreezeTimer.erase(sceneObjectEntryToRemove);
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateTextOnDamage(const strutils::StringId& damagedSceneObjectName, const glm::vec3& textOriginPos, const int damage)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Update damage count of existing text scene object
    auto existingDamageTextForSceneObjectIter = mDamagedSceneObjectNameToTextSceneObject.find(damagedSceneObjectName);
    if (existingDamageTextForSceneObjectIter != mDamagedSceneObjectNameToTextSceneObject.cend())
    {
        auto existingDamageTextSceneObjectOpt = mScene.GetSceneObject(existingDamageTextForSceneObjectIter->second);
        if (existingDamageTextSceneObjectOpt)
        {
            existingDamageTextSceneObjectOpt->get().mText = std::to_string(std::stoi(existingDamageTextSceneObjectOpt->get().mText) + damage);
            
            mDamagedSceneObjectNameToTextSceneObjectFreezeTimer[damagedSceneObjectName] = TEXT_DAMAGE_FREEZE_MILLIS;
        }
    }
    // Otherwise create a new text scene object
    else
    {
        bool enemyDamaged = damagedSceneObjectName != game_constants::PLAYER_SCENE_OBJECT_NAME;
        SceneObject damageTextSO;
        damageTextSO.mPosition = textOriginPos;
        damageTextSO.mPosition.x += TEXT_DAMAGE_X_OFFSET;
        damageTextSO.mPosition.y += TEXT_DAMAGE_Y_OFFSET;
        damageTextSO.mPosition.z = TEXT_DAMAGE_Z;
        damageTextSO.mScale = TEXT_DAMAGE_SCALE;
        damageTextSO.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        damageTextSO.mFontName = game_constants::DEFAULT_FONT_NAME;
        damageTextSO.mSceneObjectType = SceneObjectType::GUIObject;
        damageTextSO.mName = strutils::StringId(std::to_string(SDL_GetTicks64()));
        damageTextSO.mText = std::to_string(damage);
        damageTextSO.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = enemyDamaged ? ENEMY_TEXT_DAMAGE_COLOR : PLAYER_TEXT_DAMAGE_COLOR;
        
        mDamagedSceneObjectNameToTextSceneObject[damagedSceneObjectName] = damageTextSO.mName;
        mDamagedSceneObjectNameToTextSceneObjectFreezeTimer[damagedSceneObjectName] = TEXT_DAMAGE_FREEZE_MILLIS;
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

void LevelUpdater::ApplyShakeToNearlyDeadEntities(std::vector<SceneObject>& sceneObjects)
{
    for (auto& so: sceneObjects)
    {
        // Player special case
        if (so.mName == game_constants::PLAYER_SCENE_OBJECT_NAME)
        {
            auto healthRatio = GameSingletons::GetPlayerCurrentHealth()/GameSingletons::GetPlayerMaxHealth();
            if (healthRatio <= SHAKE_ENTITY_HEALTH_RATIO_THRESHOLD)
            {
                so.mBody->SetTransform(so.mBody->GetWorldCenter() + b2Vec2(math::RandomFloat(-SHAKE_ENTITY_RANDOM_MAG, SHAKE_ENTITY_RANDOM_MAG), math::RandomFloat(-SHAKE_ENTITY_RANDOM_MAG, SHAKE_ENTITY_RANDOM_MAG)), 0.0f);
            }
        }
        else if (!scene_object_utils::IsSceneObjectBossPart(so))
        {
            auto objectDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(so.mObjectFamilyTypeName);
            if (objectDefOpt)
            {
                auto healthRatio = so.mHealth/objectDefOpt->get().mHealth;
                if (healthRatio <= SHAKE_ENTITY_HEALTH_RATIO_THRESHOLD)
                {
                    so.mBody->SetTransform(so.mBody->GetWorldCenter() + b2Vec2(math::RandomFloat(-SHAKE_ENTITY_RANDOM_MAG, SHAKE_ENTITY_RANDOM_MAG), math::RandomFloat(-SHAKE_ENTITY_RANDOM_MAG, SHAKE_ENTITY_RANDOM_MAG)), 0.0f);
                }
            }
        }
    }
    
    // For boss SOs calculate a global offset
    if (mCurrentWaveNumber < mLevel.mWaves.size() && !mLevel.mWaves.at(mCurrentWaveNumber).mBossName.isEmpty() && mWaveEnemies.size() > 0 && mStateMachine.GetActiveStateName() == FightingWaveGameState::STATE_NAME)
    {
        b2Vec2 randomOffset = b2Vec2(math::RandomFloat(-SHAKE_ENTITY_RANDOM_MAG, SHAKE_ENTITY_RANDOM_MAG), math::RandomFloat(-SHAKE_ENTITY_RANDOM_MAG, SHAKE_ENTITY_RANDOM_MAG));
        
        for (auto& so: sceneObjects)
        {
            if (scene_object_utils::IsSceneObjectBossPart(so) && GameSingletons::GetBossCurrentHealth()/GameSingletons::GetBossMaxHealth() <= SHAKE_ENTITY_HEALTH_RATIO_THRESHOLD && mBossPositioned)
            {
                so.mBody->SetTransform(so.mBody->GetWorldCenter() + randomOffset, 0.0f);
            }
        }
    }
}

///------------------------------------------------------------------------------------------------
