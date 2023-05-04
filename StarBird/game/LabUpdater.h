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
#include "UpgradeUnlockedHandler.h"

#include <SDL_events.h>
#include <memory>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

class CarouselController;
class TextPromptController;
class Scene;
class b2World;
class LabUpdater final: public IUpdater
{
public:
    LabUpdater(Scene& scene, b2World& box2dWorld);
    ~LabUpdater();
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif
    
    void VOpenSettingsMenu() override;
    
private:
    void CreateSceneObjects();
    void PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const;
    void OnCarouselMovementStart();
    void OnCarouselStationary();
    void OnConfirmationButtonPressed();
    void OnTriggerOptionFlow();
    
    std::string CheckForOptionValidity() const;
    
private:
    enum class OptionSelectionState
    {
        OPTION_NOT_SELECTED, OPTION_SELECTED, OPTION_TRIGGERED, OPTION_FLOW_FINISHED, TRANSITIONING_TO_NEXT_SCREEN
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    UpgradeUnlockedHandler mUpgradeUnlockedHandler;
    std::vector<game_constants::LabOptionType> mLabOptions;
    std::unordered_set<game_constants::LabOptionType> mVisitedLabOptions;
    std::unique_ptr<TextPromptController> mTextPromptController;
    std::unique_ptr<CarouselController> mCarouselController;
    OptionSelectionState mOptionSelectionState;
    game_constants::LabOptionType mSelectedLabOption;
};


///------------------------------------------------------------------------------------------------

#endif /* LabUpdater_h */
