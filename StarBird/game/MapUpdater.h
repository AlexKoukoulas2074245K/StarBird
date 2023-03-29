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
    ~MapUpdater();
    
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void OnAppStateChange(Uint32 event) override;
    std::string GetDescription() const override;
    
#ifdef DEBUG
    void OpenDebugConsole() override;
#endif

private:
    bool SelectedActiveLevel(const glm::vec3& touchPos);
    
private:
    Scene& mScene;
    StateMachine mStateMachine;
    Map mMap;    
    MapCoord mSelectedMapCoord;
    bool mTransitioning;
};

///------------------------------------------------------------------------------------------------

#endif /* MapUpdater_h */
