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
#include "Map.h"
#include "SceneObject.h"
#include "StateMachine.h"
#include "../utils/MathUtils.h"

#include <SDL_events.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class FullScreenOverlayController;
class MapUpdater final: public IUpdater
{
public:
    MapUpdater(Scene& scene);
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif

private:
    bool CheckForActiveLevelSelection(const glm::vec3& touchPos);
    void OnLevelSelection();
    void OnLevelDeselection();
    void OnConfirmationButtonPressed();
    void OnEventNodeSelected();
    
private:
    Scene& mScene;
    StateMachine mStateMachine;
    Map mMap;
    MapCoord mCurrentMapCoord;
    MapCoord mSelectedMapCoord;
    Uint32 mLastInputContextEventType;
    bool mTransitioning;
};

///------------------------------------------------------------------------------------------------

#endif /* MapUpdater_h */
