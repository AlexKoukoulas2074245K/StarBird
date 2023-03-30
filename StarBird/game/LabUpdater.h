///------------------------------------------------------------------------------------------------
///  LabUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LabUpdater_h
#define LabUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "StateMachine.h"

#include <SDL_events.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class LabUpdater final: public IUpdater
{
public:
    LabUpdater(Scene& scene);
    ~LabUpdater();
    
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void OnAppStateChange(Uint32 event) override;
    std::string GetDescription() const override;
    
#ifdef DEBUG
    void OpenDebugConsole() override;
#endif
    
private:
    void CreateSceneObjects();
    void PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const;
    
private:
    enum class CarouselState
    {
        STATIONARY, MOVING_LEFT, MOVING_RIGHT
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    std::vector<strutils::StringId> mLabOptionSoNames;
    CarouselState mCarouselState;
    float mCarouselRads;
    float mCarouselTargetRads;
    bool mTransitioning;
};


///------------------------------------------------------------------------------------------------

#endif /* LabUpdater_h */
