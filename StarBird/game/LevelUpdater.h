///------------------------------------------------------------------------------------------------
///  LevelUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LevelUpdater_h
#define LevelUpdater_h

///------------------------------------------------------------------------------------------------

#include "SceneObject.h"
#include "LevelDefinition.h"
#include "RepeatableFlow.h"
#include "UpgradeDefinition.h"
#include "UpgradesLogicHandler.h"
#include "StateMachine.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinition;
class Scene;
class b2World;
class LevelUpdater final
{
public:
    LevelUpdater(Scene& scene, b2World& box2dWorld);
    
    void InitLevel(LevelDefinition&& levelDef);
    void OnAppStateChange(Uint32 event);
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMillis);
    void AdvanceWave();
    void AddFlow(RepeatableFlow&& flow);
    void AddWaveEnemy(const strutils::StringId& enemyTag);
    
    const LevelDefinition& GetCurrentLevelDefinition() const;
    size_t GetCurrentWaveNumber() const;
    size_t GetWaveEnemyCount() const;
    std::optional<std::reference_wrapper<RepeatableFlow>> GetFlow(const strutils::StringId& flowName);
    
private:
    void UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMillis);
    void UpdateHealthBars(const float dtMillis);
    void UpdateBackground(const float dtMillis);
    void UpdateFlows(const float dtMillis);
    
    void OnBlockedUpdate();

    void CreateBulletAtPosition(const strutils::StringId& bulletType, const glm::vec3& position);
    
private:
    Scene& mScene;
    b2World& mBox2dWorld;
    LevelDefinition mLevel;
    UpgradesLogicHandler mUpgradesLogicHandler;
    StateMachine mStateMachine;
    PostStateUpdateDirective mLastPostStateMachineUpdateDirective;
    std::vector<RepeatableFlow> mFlows;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mWaveEnemies;
    
    glm::vec3 mPreviousMotionVec;
    size_t mCurrentWaveNumber;
    bool mAllowInputControl;
    bool mMovementRotationAllowed;
};

///------------------------------------------------------------------------------------------------

#endif /* LevelUpdater_h */
