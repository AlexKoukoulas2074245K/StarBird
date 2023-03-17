///------------------------------------------------------------------------------------------------
///  IBossAI.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/03/2023
///------------------------------------------------------------------------------------------------

#ifndef IBossAI_h
#define IBossAI_h

///------------------------------------------------------------------------------------------------

#include "../../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

class Scene;
class LevelUpdater;
class StateMachine;
class b2World;
class IBossAI
{
public:
    IBossAI(Scene& scene, LevelUpdater& levelUpdater, StateMachine& stateMachine, b2World& box2dWorld)
        : mScene(scene)
        , mLevelUpdater(levelUpdater)
        , mStateMachine(stateMachine)
        , mBox2dWorld(box2dWorld) {}
        
    virtual ~IBossAI() = default;
    virtual void VUpdateBossAI(const float dtMillis) = 0;
    
protected:
    Scene& mScene;
    LevelUpdater& mLevelUpdater;
    StateMachine& mStateMachine;
    b2World& mBox2dWorld;
};

///------------------------------------------------------------------------------------------------

#endif /* IBossAI_h */
