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
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif
    
private:
    void CreateSceneObjects();
    void PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const;
    void OnCarouselMovementStart();
    void OnCarouselStationary();
    void OnConfirmationButtonPressed();
    void OnTriggerOptionFlow();
    
    std::string CheckForOptionValidity() const;
    
private:
    enum class CarouselState
    {
        STATIONARY, MOVING_LEFT, MOVING_RIGHT
    };
    
    enum class OptionSelectionState
    {
        OPTION_NOT_SELECTED, OPTION_SELECTED, OPTION_TRIGGERED, OPTION_FLOW_FINISHED, TRANSITIONING_TO_NEXT_SCREEN
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    std::vector<game_constants::LabOptionType> mLabOptions;
    std::unordered_set<game_constants::LabOptionType> mVisitedLabOptions;
    std::unique_ptr<TextPromptController> mTextPromptController;
    CarouselState mCarouselState;
    OptionSelectionState mOptionSelectionState;
    game_constants::LabOptionType mSelectedLabOption;
    float mCarouselRads;
    float mCarouselTargetRads;
};


///------------------------------------------------------------------------------------------------

#endif /* LabUpdater_h */
