///------------------------------------------------------------------------------------------------
///  ResearchUpdater.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/04/2023
///------------------------------------------------------------------------------------------------

#ifndef ResearchUpdater_h
#define ResearchUpdater_h

///------------------------------------------------------------------------------------------------

#include "RepeatableFlow.h"
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
class ResearchUpdater final: public IUpdater
{
public:
    ResearchUpdater(Scene& scene);
    ~ResearchUpdater();
    
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
    void UpdateFadeableSceneObjects(const float dtMillis);
    void UpdateUnlockBarSceneObjects(const float dtMillis);
    void CreateCrystalsTowardTargetPosition(const long crystalCount, const glm::vec3& position);
    
private:
    enum class OptionSelectionState
    {
        OPTION_NOT_SELECTED, EXPEND_CRYSTALS, UNLOCK_SHAKE, UNLOCK_TEXTURE_TRANSITION, TRANSITIONING_TO_NEXT_SCREEN
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    std::vector<RepeatableFlow> mFlows;
    std::vector<strutils::StringId> mUpgrades;
    std::vector<strutils::StringId> mFadeableSceneObjects;
    std::vector<strutils::StringId> mCrystalSceneObjectNames;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mVisitedUpgrades;
    std::unique_ptr<CarouselController> mCarouselController;
    OptionSelectionState mOptionSelectionState;
    strutils::StringId mSelectedUpgrade;
    long mCurrentOperationCrystalCost;
    float mOptionShakeMagnitude;
    bool mCarouselMoving;
};


///------------------------------------------------------------------------------------------------

#endif /* ResearchUpdater_h */
