///------------------------------------------------------------------------------------------------
///  StateMachine.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef StateMachine_h
#define StateMachine_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"
#include "../../utils/StringUtils.h"

#include <memory>
#include <unordered_map>
#include <type_traits>

///------------------------------------------------------------------------------------------------

class Scene;
class LevelUpdater;
class b2World;
class UpgradesLogicHandler;
class StateMachine final
{
public:
    StateMachine(Scene& scene, LevelUpdater& levelUpdater, UpgradesLogicHandler& upgradesLogicHandler, b2World& box2dWorld)
        : mScene(scene)
        , mLevelUpdater(levelUpdater)
        , mUpgradesLogicHandler(upgradesLogicHandler)
        , mBox2dWorld(box2dWorld)
        , mCurrentState(nullptr)
    {
    }
    
    template <class StateClass>
    void RegisterState()
    {
        static_assert(std::is_base_of<BaseGameState, StateClass>::value);
        auto stateInstance = std::make_unique<StateClass>();
        stateInstance->SetDependencies(&mScene, &mLevelUpdater, &mUpgradesLogicHandler, &mBox2dWorld);
        mStateNameToInstanceMap[StateClass::STATE_NAME] = std::move(stateInstance);
    }
    
    void InitStateMachine(const strutils::StringId& initStateName);
    PostStateUpdateDirective Update(const float dtMillis);
    
private:
    void SwitchToState(const strutils::StringId& nextStateName);
    
private:
    Scene& mScene;
    LevelUpdater& mLevelUpdater;
    UpgradesLogicHandler& mUpgradesLogicHandler;
    b2World& mBox2dWorld;
    
    std::unordered_map<strutils::StringId, std::unique_ptr<BaseGameState>, strutils::StringIdHasher> mStateNameToInstanceMap;
    BaseGameState* mCurrentState;
    
};

///------------------------------------------------------------------------------------------------

#endif /* StateMachine_h */
