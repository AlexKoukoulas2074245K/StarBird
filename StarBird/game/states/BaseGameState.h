///------------------------------------------------------------------------------------------------
///  BaseGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BaseGameState_h
#define BaseGameState_h

///------------------------------------------------------------------------------------------------

#include "../../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

enum class PostStateUpdateDirective
{
    CONTINUE,
    BLOCK_UPDATE
};

///------------------------------------------------------------------------------------------------

class Scene;
class LevelUpdater;
class UpgradesLevelLogicHandler;
class b2World;
class BaseGameState
{
public:
    friend class StateMachine;
    
    BaseGameState()
        : mScene(nullptr)
        , mLevelUpdater(nullptr)
        , mUpgradesLogicHandler(nullptr)
        , mBox2dWorld(nullptr)
    {
    }
    
    virtual void VInitialize() {}
    virtual PostStateUpdateDirective VUpdate(const float dtMillis) { return PostStateUpdateDirective::CONTINUE; }
    virtual void VDestroy() {}
    
    bool IsComplete() const { return !mNextStateName.isEmpty(); }
    const strutils::StringId& GetNextStateName() const { return mNextStateName; }
    
    void SetDependencies(Scene* scene, LevelUpdater* levelUpdater, UpgradesLevelLogicHandler* upgradesLogicHandler, b2World* world)
    {
        mScene = scene;
        mLevelUpdater = levelUpdater;
        mUpgradesLogicHandler = upgradesLogicHandler;
        mBox2dWorld = world;
    }
    
protected:
    static const strutils::StringId POP_STATE_COMPLETION_NAME;
    
    void Complete()
    {
        mNextStateName = POP_STATE_COMPLETION_NAME;
    }
    
    void Complete(const strutils::StringId& nextStateName)
    {
        mNextStateName = nextStateName;
    }
    
protected:
    Scene* mScene;
    LevelUpdater* mLevelUpdater;
    UpgradesLevelLogicHandler* mUpgradesLogicHandler;
    b2World* mBox2dWorld;
    
private:
    strutils::StringId mNextStateName = strutils::StringId();
};

///------------------------------------------------------------------------------------------------

#endif /* BaseGameState_h */
