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
#include "StateMachine.h"
#include "../utils/MathUtils.h"

#include <SDL_events.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class StatsUpgradeUpdater final: public IUpdater
{
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
    
private:
    enum class SelectionState
    {
        NO_STATS_SELECTED,
        TRANSITIONING_TO_NEXT_SCREEN
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    SelectionState mSelectionState;
};


///------------------------------------------------------------------------------------------------

#endif /* StatsUpgradeUpdater_h */
