///------------------------------------------------------------------------------------------------
///  StatsUpgradeUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef StatsUpgradeUpdater_h
#define StatsUpgradeUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "RepeatableFlow.h"
#include "StateMachine.h"
#include "StatUpgradeAreaController.h"
#include "../utils/MathUtils.h"

#include <SDL_events.h>
#include <memory>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class Scene;
class StatsUpgradeUpdater final: public IUpdater
{
public:
    enum class StatType
    {
        ATTACK_STAT = 0,
        SPEED_STAT = 1,
        HASTE_STAT = 2,
        HEALTH_STAT = 3,
        COUNT = 4
    };
    
public:
    StatsUpgradeUpdater(Scene& scene);
    ~StatsUpgradeUpdater();
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif

private:
    void CreateSceneObjects();
    void CreateCrystalsTowardTargetPosition(const int crystalCount, const glm::vec3& position);
    void OnConfirmationButtonPressed();
    
private:
    enum class SelectionState
    {
        NO_STATS_SELECTED,
        ONE_OR_MORE_STATS_HAVE_BEEN_SELECTED,
        EXPENDING_CRYSTALS,
        TRANSITIONING_TO_NEXT_SCREEN
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    SelectionState mSelectionState;
    std::vector<strutils::StringId> mCrystalSceneObjectNames;
    std::unordered_map<StatType, std::unique_ptr<StatUpgradeAreaController>> mStatControllers;
    std::vector<RepeatableFlow> mFlows;
};


///------------------------------------------------------------------------------------------------

#endif /* StatsUpgradeUpdater_h */
