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
#include <unordered_set>

///------------------------------------------------------------------------------------------------

class TextPromptController;
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
    void OnCarouselMovementStart();
    void OnCarouselStationary();
    void OnConfirmationButtonPress();
    
private:
    enum class CarouselState
    {
        STATIONARY, MOVING_LEFT, MOVING_RIGHT
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    std::vector<game_constants::LabOptionType> mLabOptions;
    std::unordered_set<game_constants::LabOptionType> mVisitedLabOptions;
    std::unique_ptr<TextPromptController> mTextPromptController;
    CarouselState mCarouselState;
    game_constants::LabOptionType mSelectedLabOption;
    float mCarouselRads;
    float mCarouselTargetRads;
    bool mTransitioning;
};


///------------------------------------------------------------------------------------------------

#endif /* LabUpdater_h */
