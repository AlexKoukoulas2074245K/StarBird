///------------------------------------------------------------------------------------------------
///  KathunBossAI.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "KathunBossAI.h"
#include "../LevelUpdater.h"
#include "../Scene.h"
#include "../GameConstants.h"
#include "../GameSingletons.h"
#include "../datarepos/ObjectTypeDefinitionRepository.h"
#include "../SceneObjectUtils.h"
#include "../states/StateMachine.h"
#include "../states/FightingWaveGameState.h"
#include "../../utils/Logging.h"

#include <vector>

///------------------------------------------------------------------------------------------------

const strutils::StringId KathunBossAI::BOSS_NAME = strutils::StringId("Ka'thun");

///------------------------------------------------------------------------------------------------

const std::unordered_map<KathunBossAI::Ability, std::unordered_map<KathunBossAI::State, float>> KathunBossAI::ABILITY_COOLDOWNS_PER_STATE =
{
    { KathunBossAI::Ability::SPAWN_CHASER, {{ KathunBossAI::State::PHASE_1, 4000.0f }, { KathunBossAI::State::PHASE_2, 4000.0f }, { KathunBossAI::State::PHASE_3, 4000.0f }} },
    { KathunBossAI::Ability::VERTICAL_BULLET, {{ KathunBossAI::State::PHASE_1, 1000.0f }, { KathunBossAI::State::PHASE_2, 500.0f }, { KathunBossAI::State::PHASE_3, 400.0f }} },
    { KathunBossAI::Ability::DIAGONAL_BULLET, {{ KathunBossAI::State::PHASE_2, 3000.0f }, { KathunBossAI::State::PHASE_3, 2000.0f }} },
    { KathunBossAI::Ability::INSTA_DEATH, {{ KathunBossAI::State::PHASE_3, 20000.0f }} }
};

const std::unordered_map<KathunBossAI::State, float> KathunBossAI::MIN_HEALTH_PERCENTAGE_PER_STATE =
{
    { KathunBossAI::State::PHASE_1, 0.80f },
    { KathunBossAI::State::PHASE_2, 0.40f },
    { KathunBossAI::State::PHASE_3, 0.00001f }
};

///------------------------------------------------------------------------------------------------

static const float KATHUN_SET_Y = 7.0f;
static const std::string KATHUN_ABILITY_FLOW_NAME_POST_FIX = "_ABILITY_FLOW";
static const strutils::StringId KATHUN_BODY_NAME = strutils::StringId("enemies/boss_1/body");
static const strutils::StringId KATHUN_SLOW_CHASER_ENEMY_TYPE = strutils::StringId("enemies/medium_enemy_chasing");
static const strutils::StringId KATHUN_FAST_CHASER_ENEMY_TYPE = strutils::StringId("enemies/small_enemy_chasing");
static const strutils::StringId KATHUN_BULLET_TYPE = strutils::StringId("enemies/boss_1/bullet");

static const std::vector<strutils::StringId> KATHUN_FLAP_NAMES =
{
    strutils::StringId("enemies/boss_1/top_left_flap"),
    strutils::StringId("enemies/boss_1/middle_left_flap"),
    strutils::StringId("enemies/boss_1/bottom_left_flap"),
    strutils::StringId("enemies/boss_1/top_right_flap"),
    strutils::StringId("enemies/boss_1/middle_right_flap"),
    strutils::StringId("enemies/boss_1/bottom_right_flap")
};

static const std::vector<glm::vec3> KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS =
{
    glm::vec3(0.0f, 0.0f,  -0.0f),
    glm::vec3(-3.0f, 0.0f, -0.5f),
    glm::vec3(-5.0f, 0.0f, -0.5f),
    glm::vec3(+3.0f, 0.0f, -0.5f),
    glm::vec3(+5.0f, 0.0f, -0.5f)
};

///------------------------------------------------------------------------------------------------

KathunBossAI::KathunBossAI(Scene& scene, LevelUpdater& levelUpdater, StateMachine& stateMachine, b2World& box2dWorld)
    : IBossAI(scene, levelUpdater, stateMachine, box2dWorld)
    , mState(State::BOSS_MOVING_TO_POSITION)
    , mShaking(false)
{
    ObjectTypeDefinitionRepository::GetInstance().LoadObjectTypeDefinition(KATHUN_SLOW_CHASER_ENEMY_TYPE);
    ObjectTypeDefinitionRepository::GetInstance().LoadObjectTypeDefinition(KATHUN_FAST_CHASER_ENEMY_TYPE);
    ObjectTypeDefinitionRepository::GetInstance().LoadObjectTypeDefinition(KATHUN_BULLET_TYPE);
}

///------------------------------------------------------------------------------------------------

void KathunBossAI::VUpdateBossAI(const float dtMillis)
{
    switch (mState)
    {
        case State::BOSS_MOVING_TO_POSITION:
        {
            auto bodySoOpt = mScene.GetSceneObject(KATHUN_BODY_NAME);
            if (bodySoOpt)
            {
                auto& bossSo = bodySoOpt->get();
                if (bossSo.mBody->GetWorldCenter().y <= KATHUN_SET_Y)
                {
                    mLevelUpdater.OnBossPositioned();
                    mState = State::BOSS_POSITIONED;
                    OnStateChange(true);
                }
                else
                {
                    bossSo.mInvulnerable = true;
                    std::for_each(KATHUN_FLAP_NAMES.cbegin(), KATHUN_FLAP_NAMES.cend(), [&](const strutils::StringId& name)
                    {
                        mScene.GetSceneObject(name)->get().mInvulnerable = true;
                    });
                }
            }
        } break;
        
        case State::BOSS_POSITIONED:
        {
            auto bodySoOpt = mScene.GetSceneObject(KATHUN_BODY_NAME);
            auto& bossSo = bodySoOpt->get();
            bossSo.mCustomDrivenMovement = true;
            bossSo.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            
            std::for_each(KATHUN_FLAP_NAMES.cbegin(), KATHUN_FLAP_NAMES.cend(), [&](const strutils::StringId& name)
            {
                mScene.GetSceneObject(name)->get().mCustomDrivenMovement = true;
                mScene.GetSceneObject(name)->get().mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            });
            
            if (mStateMachine.GetActiveStateName() == FightingWaveGameState::STATE_NAME)
            {
                mState = State::PHASE_1;
                OnStateChange(false);
            }
        } break;
            
        case State::PHASE_1:
        case State::PHASE_2:
        case State::PHASE_3:
        {
            auto bodySoOpt = mScene.GetSceneObject(KATHUN_BODY_NAME);
            
            if (bodySoOpt)
            {
                auto& bossSo = bodySoOpt->get();
                bossSo.mInvulnerable = false;
            }
            
            std::for_each(KATHUN_FLAP_NAMES.cbegin(), KATHUN_FLAP_NAMES.cend(), [&](const strutils::StringId& name)
            {
                auto soOpt = mScene.GetSceneObject(name);
                if (soOpt)
                {
                    soOpt->get().mInvulnerable = false;
                }
            });
            
            const auto currentBossHealthPerc = GameSingletons::GetBossCurrentHealth()/GameSingletons::GetBossMaxHealth();
            auto currentStateMinHealthPerc = MIN_HEALTH_PERCENTAGE_PER_STATE.at(mState);
            
            // If we cross the health threshold for next phase change to it
            if (currentBossHealthPerc < currentStateMinHealthPerc)
            {
                mState = static_cast<State>(static_cast<int>(mState) + 1);
                
                // Handle boss death
                if (mState == State::COUNT)
                {
                    for (auto i = 0; i < static_cast<int>(Ability::COUNT); ++i)
                    {
                        auto flowOpt = mLevelUpdater.GetFlow(strutils::StringId(std::to_string(i) + KATHUN_ABILITY_FLOW_NAME_POST_FIX));
                        if (flowOpt)
                        {
                            flowOpt->get().ForceFinish();
                        }
                    }
                }
                else
                {
                    OnStateChange(true);
                }
            }
            
        } break;
            
        default: break;
    }
    
    if (mShaking)
    {
        CameraShake();
    }
}

///------------------------------------------------------------------------------------------------

void KathunBossAI::OnAbilityTrigger(const Ability ability)
{
    
    auto bodySoOpt = mScene.GetSceneObject(KATHUN_BODY_NAME);
    auto& bossSo = bodySoOpt->get();
    auto bossPosition = math::Box2dVec2ToGlmVec3(bossSo.mBody->GetWorldCenter());
    
    switch (mState)
    {
        case State::PHASE_1:
        {
            switch (ability)
            {
                case Ability::SPAWN_CHASER:
                {
                    SpawnEnemyAt(bossPosition + glm::vec3(0.0f, 0.0f, -0.5f) + glm::vec3(math::RandomFloat(-5.0f, 5.0f), 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_SLOW_CHASER_ENEMY_TYPE);
                } break;
                
                case Ability::VERTICAL_BULLET:
                {
                    float randomXOffset = math::RandomFloat(-0.4, 0.4f);
                    glm::vec3 randomOffset = glm::vec3(randomXOffset, 0.0f, 0.0f);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[math::RandomInt(0, static_cast<int>(KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS.size()) - 1)] + randomOffset, glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_BULLET_TYPE);
                } break;
                    
                default: break;
            }
        } break;
            
        case State::PHASE_2:
        {
            switch (ability)
            {
                case Ability::SPAWN_CHASER:
                {
                    SpawnEnemyAt(bossPosition + glm::vec3(-2.0f, 0.0f, -0.5f) + glm::vec3(math::RandomFloat(-5.0f, 5.0f), 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_FAST_CHASER_ENEMY_TYPE);
                    SpawnEnemyAt(bossPosition + glm::vec3(+2.0f, 0.0f, -0.5f) + glm::vec3(math::RandomFloat(-5.0f, 5.0f), 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_FAST_CHASER_ENEMY_TYPE);
                } break;
                
                case Ability::VERTICAL_BULLET:
                {
                    float randomXOffset = math::RandomFloat(-0.4, 0.4f);
                    glm::vec3 randomOffset = glm::vec3(randomXOffset, 0.0f, 0.0f);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[math::RandomInt(0, static_cast<int>(KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS.size()) - 1)] + randomOffset, glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_BULLET_TYPE);
                } break;
                    
                case Ability::DIAGONAL_BULLET:
                {
                    const auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(-worldCamera.GetCameraLenseWidth()/2, -worldCamera.GetCameraLenseHeight()/2, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(0.0f, 0.0f, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(worldCamera.GetCameraLenseWidth()/2, -worldCamera.GetCameraLenseHeight()/2, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                } break;
                
                default: break;
            }
        } break;
            
        case State::PHASE_3:
        {
            switch (ability)
            {
                case Ability::SPAWN_CHASER:
                {
                    SpawnEnemyAt(bossPosition + glm::vec3(0.0f, 0.0f, -0.5f) + glm::vec3(math::RandomFloat(-5.0f, 5.0f), 0.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_SLOW_CHASER_ENEMY_TYPE);
                    SpawnEnemyAt(bossPosition + glm::vec3(-2.0f, 0.0f, -0.5f) + glm::vec3(math::RandomFloat(-5.0f, 5.0f), 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_FAST_CHASER_ENEMY_TYPE);
                    SpawnEnemyAt(bossPosition + glm::vec3(+2.0f, 0.0f, -0.5f) + glm::vec3(math::RandomFloat(-5.0f, 5.0f), 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_FAST_CHASER_ENEMY_TYPE);
                } break;
                
                case Ability::VERTICAL_BULLET:
                {
                    float randomXOffset = math::RandomFloat(-0.4, 0.4f);
                    glm::vec3 randomOffset = glm::vec3(randomXOffset, 0.0f, 0.0f);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[math::RandomInt(0, static_cast<int>(KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS.size()) - 1)] + randomOffset, glm::vec3(0.0f, 0.0f, 0.0f), KATHUN_BULLET_TYPE);
                } break;
                
                case Ability::DIAGONAL_BULLET:
                {
                    const auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(-worldCamera.GetCameraLenseWidth()/2, -worldCamera.GetCameraLenseHeight()/2, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(-worldCamera.GetCameraLenseWidth()/4, -worldCamera.GetCameraLenseHeight()/2, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(0.0f, 0.0f, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(worldCamera.GetCameraLenseWidth()/4, -worldCamera.GetCameraLenseHeight()/2, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                    SpawnEnemyAt(bossPosition + KATHUN_VERTICAL_BULLET_SPAWN_POSITIONS[0], glm::normalize(glm::vec3(worldCamera.GetCameraLenseWidth()/2, -worldCamera.GetCameraLenseHeight()/2, -0.5f) - bossPosition), KATHUN_BULLET_TYPE);
                } break;
                    
                case Ability::INSTA_DEATH:
                {
                    
                } break;
                    
                default: break;
            }
        } break;
            
        default: break;
    }
}

///------------------------------------------------------------------------------------------------

void KathunBossAI::OnStateChange(const bool shakeCamera)
{
    for (const auto& abilityStateMapEntry: ABILITY_COOLDOWNS_PER_STATE)
    {
        // If flow for ability already exists, sets its duration to new state value
        auto flowName = strutils::StringId(std::to_string(static_cast<int>(abilityStateMapEntry.first)) + KATHUN_ABILITY_FLOW_NAME_POST_FIX);
        auto flowOpt = mLevelUpdater.GetFlow(flowName);
        if (flowOpt && abilityStateMapEntry.second.count(mState))
        {
            flowOpt->get().SetDuration(abilityStateMapEntry.second.at(mState));
        }
        // Otherwise create a brand new flow
        else if (abilityStateMapEntry.second.count(mState))
        {
            mLevelUpdater.AddFlow(RepeatableFlow([&]()
            {
                OnAbilityTrigger(abilityStateMapEntry.first);
            }, abilityStateMapEntry.second.at(mState), RepeatableFlow::RepeatPolicy::REPEAT, flowName));
        }
    }
    
    if (shakeCamera)
    {
        mShaking = true;
        for (const auto& flapName: KATHUN_FLAP_NAMES)
        {
            auto soOpt = mScene.GetSceneObject(flapName);
            if (soOpt)
            {
                static_cast<RotationAnimation*>(soOpt->get().mAnimation.get())->SetRotationMode(RotationAnimation::RotationMode::ROTATE_TO_TARGET_ONCE);
            }
        }
        mLevelUpdater.AddFlow(RepeatableFlow([&]()
        {
            mShaking = false;
            for (const auto& flapName: KATHUN_FLAP_NAMES)
            {
                auto soOpt = mScene.GetSceneObject(flapName);
                if (soOpt)
                {
                    static_cast<RotationAnimation*>(soOpt->get().mAnimation.get())->SetRotationMode(RotationAnimation::RotationMode::ROTATE_TO_TARGET_AND_BACK_CONTINUALLY);
                }
            }
        }, game_constants::BOSS_INTRO_DURATION_MILLIS/2, RepeatableFlow::RepeatPolicy::ONCE));
    }
}

///------------------------------------------------------------------------------------------------

void KathunBossAI::CameraShake()
{
    const auto& guiCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    
    if (guiCamOpt) guiCamOpt->get().Shake();
    if (worldCamOpt) worldCamOpt->get().Shake();
}

///------------------------------------------------------------------------------------------------

void KathunBossAI::SpawnEnemyAt(const glm::vec3& position, const glm::vec3& direction, const strutils::StringId& enemyType)
{
    auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    SceneObject so = scene_object_utils::CreateSceneObjectWithBody(objectTypeDefRepo.GetObjectTypeDefinition(enemyType)->get(), position, mBox2dWorld);
    auto enemyName = so.mName;
    
    // Diagonal bullet handling
    if (glm::length(direction) > 0.0f)
    {
        auto speed = -objectTypeDefRepo.GetObjectTypeDefinition(so.mObjectFamilyTypeName)->get().mConstantLinearVelocity.y;
        so.mRotation.z = -math::Arctan2(direction.x, direction.y) + math::PI;
        so.mCustomDrivenMovement = true;
        so.mBody->SetLinearVelocity(b2Vec2(direction.x * speed, direction.y * speed));
    }
    
    mLevelUpdater.AddWaveEnemy(enemyName);
    mScene.AddSceneObject(std::move(so));
}

///------------------------------------------------------------------------------------------------
