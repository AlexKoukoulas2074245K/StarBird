///------------------------------------------------------------------------------------------------
///  KathunBossAI.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/03/2023
///------------------------------------------------------------------------------------------------

#ifndef KathunBossAI_h
#define KathunBossAI_h

///------------------------------------------------------------------------------------------------

#include "IBossAI.h"
#include "../../utils/MathUtils.h"

#include <unordered_map>

///------------------------------------------------------------------------------------------------

class Scene;
class LevelUpdater;
class StateMachine;
class b2World;
class ObjectTypeDefinition;
class KathunBossAI final : public IBossAI
{
public:
    static const strutils::StringId BOSS_NAME;

public:
    KathunBossAI(Scene& scene, LevelUpdater& levelUpdater, StateMachine& stateMachine, b2World& box2dWorld);
    
    void VUpdateBossAI(const float dtMillis) override;
    
private:
    enum class State
    {
        BOSS_MOVING_TO_POSITION = 0,
        BOSS_POSITIONED = 1,
        PHASE_1 = 2,
        PHASE_2 = 3,
        PHASE_3 = 4,
        COUNT = 5
    };
    
    enum class Ability
    {
        SPAWN_CHASER = 0,
        VERTICAL_BULLET = 1,
        DIAGONAL_BULLET = 2,
        INSTA_DEATH = 3,
        COUNT = 4
    };
    
    static const std::unordered_map<Ability, std::unordered_map<State, float>> ABILITY_COOLDOWNS_PER_STATE;
    static const std::unordered_map<State, float> MIN_HEALTH_PERCENTAGE_PER_STATE;
    
private:
    void OnAbilityTrigger(const Ability ability);
    void OnStateChange(const bool shakeCamera);
    void CameraShake();
    void SpawnEnemyAt(const glm::vec3& position, const glm::vec3& direction, const strutils::StringId& enemyType);
    
private:
    State mState;
    bool mShaking;
};

///------------------------------------------------------------------------------------------------

#endif /* KathunBossAI_h */
