///------------------------------------------------------------------------------------------------
///  MapUpdater.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef MapUpdater_h
#define MapUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "SceneObject.h"
#include "StateMachine.h"

#include <SDL_events.h>
#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;
class MapUpdater final: public IUpdater
{
public:
    MapUpdater(Scene& scene);
    
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void OnAppStateChange(Uint32 event) override;
    std::string GetDescription() const override;
    
#ifdef DEBUG
    void OpenDebugConsole() override;
#endif

private:
    Scene& mScene;
    StateMachine mStateMachine;
};

///------------------------------------------------------------------------------------------------

#endif /* MapUpdater_h */
