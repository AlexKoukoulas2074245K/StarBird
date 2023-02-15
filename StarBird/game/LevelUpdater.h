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
#include "../utils/StringUtils.h"

#include <optional>
#include <unordered_set>
#include <vector>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinition;
class Scene;
class b2World;
class LevelUpdater final
{
public:
    LevelUpdater(Scene& scene, b2World& box2dWorld);
    
    void InitLevel(LevelDefinition&& levelDef);
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMillis);
    
    size_t GetWaveEnemyCount() const;
    std::optional<std::reference_wrapper<RepeatableFlow>> GetFlow(const strutils::StringId& flowName);
    
private:
    enum class LevelState
    {
        WAVE_INTRO,
        FIGHTING_WAVE,
        UPGRADE_OVERLAY_IN,
        UPGRADE,
        UPGRADE_OVERLAY_OUT,
        FINISHED_LEVEL
    };
    
    enum class StateMachineUpdateResult
    {
        CONTINUE,
        BLOCK_UPDATE
    };
    
private:
    StateMachineUpdateResult UpdateStateMachine(const float dtMillis);
    void UpdateAnimation(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMillis);
    void UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMillis);
    void UpdateHealthBars(const float dtMillis);
    void UpdateBackground(const float dtMillis);
    void UpdateFlows(const float dtMillis);
    
    void OnBlockedUpdate();
    
    void CreateWaveIntro();
    void CreateWave();
    void CreateUpgradeSceneObjects();
    void CreateBulletAtPosition(const strutils::StringId& bulletType, const glm::vec3& position);
    
    strutils::StringId TestForUpgradeSelected() const;
    
private:
    Scene& mScene;
    b2World& mBox2dWorld;
    LevelDefinition mLevel;
    UpgradesLogicHandler mUpgradesLogicHandler;
    
    std::vector<RepeatableFlow> mFlows;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mWaveEnemies;
    std::pair<UpgradeDefinition, UpgradeDefinition> mUpgradeSelection;
    
    size_t mCurrentWaveNumber;
    LevelState mState;
    bool mAllowInputControl;
};

///------------------------------------------------------------------------------------------------

#endif /* LevelUpdater_h */
