///------------------------------------------------------------------------------------------------
///  LevelUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LevelUpdater_h
#define LevelUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "BossAIController.h"
#include "SceneObject.h"
#include "LevelDefinition.h"
#include "RepeatableFlow.h"
#include "UpgradeDefinition.h"
#include "UpgradesLevelLogicHandler.h"
#include "StateMachine.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinition;
class Scene;
class b2World;
class Camera;
class LevelUpdater final: public IUpdater
{
public:
    LevelUpdater(Scene& scene, b2World& box2dWorld, LevelDefinition&& levelDef);
    
    void VOnAppStateChange(Uint32 event) override;
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif
    
    void AdvanceWave();
    void AddFlow(RepeatableFlow&& flow);
    void AddWaveEnemy(const strutils::StringId& enemyName);
    void RemoveWaveEnemy(const strutils::StringId& enemyName);
    void DropCrystals(const glm::vec3& deathPosition, const float enemyDeathAnimationMillis, float crystalYieldValue);
    
    const LevelDefinition& GetCurrentLevelDefinition() const;
    bool LevelFinished() const;
    size_t GetCurrentWaveNumber() const;
    size_t GetWaveEnemyCount() const;
    std::optional<std::reference_wrapper<RepeatableFlow>> GetFlow(const strutils::StringId& flowName);
    const std::unordered_set<strutils::StringId, strutils::StringIdHasher>& GetWaveEnemyNames() const;
    void OnBossPositioned();
    
    void CreateLevelWalls(const Camera& cam, const bool invisible);
    
private:
    void LoadLevelInvariantObjects();
    
    void UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMillis);
    void UpdateBackground(const float dtMillis);
    void UpdateBossHealthBar(const float dtMillis);
    void UpdateFlows(const float dtMillis);
    void UpdateCameras(const float dtMillis);
    void UpdateLights(const float dtMillis);
    void UpdateTextDamage(const float dtMillis);
    
    void CreateTextOnDamage(const strutils::StringId& damagedSceneObjectName, const glm::vec3& textOriginPos, const int damage);
    void OnPlayerDamaged();
    void OnBlockedUpdate();
    
    void ApplyShakeToNearlyDeadEntities(std::vector<SceneObject>& sceneObjects);
    
private:
    Scene& mScene;
    b2World& mBox2dWorld;
    LevelDefinition mLevel;
    UpgradesLevelLogicHandler mUpgradesLogicHandler;
    StateMachine mStateMachine;
    BossAIController mBossAIController;
    PostStateUpdateDirective mLastPostStateMachineUpdateDirective;
    std::vector<RepeatableFlow> mFlows;
    std::unordered_map<strutils::StringId, strutils::StringId, strutils::StringIdHasher> mDamagedSceneObjectNameToTextSceneObject;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mDamagedSceneObjectNameToTextSceneObjectFreezeTimer;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mWaveEnemies;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mActiveLightNames;
    
    glm::vec3 mPreviousMotionVec;
    glm::vec2 mAccelerometerCalibrationValues;
    size_t mCurrentWaveNumber;
    float mBossAnimatedHealthBarPerc;
    bool mAllowInputControl;
    bool mMovementRotationAllowed;
    bool mBossPositioned;
};

///------------------------------------------------------------------------------------------------

#endif /* LevelUpdater_h */
