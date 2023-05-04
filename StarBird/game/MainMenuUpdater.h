///------------------------------------------------------------------------------------------------
///  MainMenuUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef MainMenuUpdater_h
#define MainMenuUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "SceneObject.h"
#include "StateMachine.h"
#include "../utils/MathUtils.h"

#include <SDL_events.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class MainMenuUpdater final: public IUpdater
{
public:
    MainMenuUpdater(Scene& scene);
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif
    void VOpenSettingsMenu() override {};
    
private:
    void UpdateBackground(const float dtMillis);
    void CreateSceneObjects();
    
private:
    Scene& mScene;
    StateMachine mStateMachine;
    bool mTransitioning;
};

///------------------------------------------------------------------------------------------------

#endif /* MainMenuUpdater_h */
