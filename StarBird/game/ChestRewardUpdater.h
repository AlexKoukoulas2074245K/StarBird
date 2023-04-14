///------------------------------------------------------------------------------------------------
///  ChestRewardUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ChestRewardUpdater_h
#define ChestRewardUpdater_h

///------------------------------------------------------------------------------------------------

#include "IUpdater.h"
#include "StateMachine.h"

#include <memory>

///------------------------------------------------------------------------------------------------

class Scene;
class FullScreenOverlayController;
class CarouselController;
class ChestRewardUpdater final: public IUpdater
{
public:
    ChestRewardUpdater(Scene& scene);
    ~ChestRewardUpdater();
    
    PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) override;
    void VOnAppStateChange(Uint32 event) override;
    std::string VGetDescription() const override;
    strutils::StringId VGetStateMachineActiveStateName() const override;
    
#ifdef DEBUG
    void VOpenDebugConsole() override;
#endif
    
private:
    enum class RewardFlowState
    {
        AWAIT_PRESS,
        SHAKE,
        OPEN_ANIMATION,
        REWARD_SELECTION
    };
    
    Scene& mScene;
    StateMachine mStateMachine;
    RewardFlowState mRewardFlowState;
    float mShakeNoiseMag;
    float mChestPulseValueAccum;
    float mChestAnimationTweenValue;
    float mChestLightDtAccum;
    
    std::unique_ptr<FullScreenOverlayController> mScreenOverlayController;
    std::unique_ptr<CarouselController> mCarouselController;
};

///------------------------------------------------------------------------------------------------

#endif /* ChestRewardUpdater_h */
