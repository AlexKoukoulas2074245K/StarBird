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
#include "RepeatableFlow.h"
#include "SceneObject.h"
#include "StateMachine.h"
#include "UpgradeUnlockedHandler.h"
#include "../utils/MathUtils.h"

#include <SDL_events.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class TextPromptController;
class b2World;
class EventUpdater final: public IUpdater
{
public:
    EventUpdater(Scene& scene, b2World& box2dWorld);
    ~EventUpdater();
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif

private:
    class EventOption
    {
    public:
        EventOption(const std::string& optionText, const size_t nextStateIndex, std::function<void()> selectionCallback)
            : mOptionText(optionText)
            , mNextStateIndex(nextStateIndex)
            , mSelectionCallback(selectionCallback)
            
        {
        }
        
    public:
        const std::string mOptionText;
        const std::function<void()> mSelectionCallback;
        const size_t mNextStateIndex;
    };
    
    class EventDescription
    {
    public:
        EventDescription(const std::vector<std::string>& eventBackgroundTextureNames, const std::vector<std::string>& eventDescriptionTexts, const std::vector<std::vector<EventOption>>& eventOptions, std::function<bool()> eventEligibilityFunc)
            : mEventBackgroundTextureNames(eventBackgroundTextureNames)
            , mEventDescriptionTexts(eventDescriptionTexts)
            , mEventOptions(eventOptions)
            , mEventEligibilityFunc(eventEligibilityFunc)
        {
        }
        
        inline size_t GetStateCount() const { return mEventDescriptionTexts.size(); }
        
    public:
        const std::vector<std::string> mEventBackgroundTextureNames;
        const std::vector<std::string> mEventDescriptionTexts;
        const std::vector<std::vector<EventOption>> mEventOptions;
        const std::function<bool()> mEventEligibilityFunc;
    };
    
private:
    void RegisterEvents();
    void SelectRandomEligibleEvent();
    void CreateSceneObjects();
    void CreateEventSceneObjectsForCurrentState();
    void CreateCrystalsTowardTargetPosition(const long crystalCount, const glm::vec3& position);
    
private:
    Scene& mScene;
    StateMachine mStateMachine;
    UpgradeUnlockedHandler mUpgradeUnlockedHandler;
    EventDescription* mSelectedEvent;
    size_t mEventProgressionStateIndex;
    size_t mPreviousEventProgressionStateIndex;
    bool mTransitioning;
    bool mFadeInOptions;
    bool mEventCompleted;
    std::unique_ptr<TextPromptController> mTextPromptController;
    std::vector<EventDescription> mRegisteredEvents;
    std::vector<RepeatableFlow> mFlows;
};

///------------------------------------------------------------------------------------------------

#endif /* EventUpdater_h */
