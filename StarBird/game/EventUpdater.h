///------------------------------------------------------------------------------------------------
///  EventUpdater.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/05/2023
///------------------------------------------------------------------------------------------------

#ifndef EventUpdater_h
#define EventUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "SceneObject.h"
#include "StateMachine.h"
#include "../utils/MathUtils.h"

#include <SDL_events.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class b2World;
class EventUpdater final: public IUpdater
{
public:
    EventUpdater(Scene& scene, b2World& box2dWorld);
    
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
    Scene& mScene;
    StateMachine mStateMachine;
    bool mTransitioning;
};

///------------------------------------------------------------------------------------------------

#endif /* EventUpdater_h */
