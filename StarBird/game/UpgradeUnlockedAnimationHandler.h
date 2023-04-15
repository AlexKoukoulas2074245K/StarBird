///------------------------------------------------------------------------------------------------
///  UpgradeUnlockedAnimationHandler.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradeUnlockedAnimationHandler_h
#define UpgradeUnlockedAnimationHandler_h

///------------------------------------------------------------------------------------------------

#include "../utils/StringUtils.h"
#include "RepeatableFlow.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;

///------------------------------------------------------------------------------------------------

class UpgradeUnlockedAnimationHandler final
{
public:
    enum class UpgradeAnimationState
    {
        ONGOING, FINISHED
    };
    
    UpgradeUnlockedAnimationHandler(Scene& scene);
    
    void OnUpgradeGained(const strutils::StringId& upgradeId);
    UpgradeAnimationState Update(const float dtMillis);

private:
    void OnCrystalGiftUpgradeGained();
    UpgradeAnimationState UpdateCrystalGiftUpgradeGained();
    
private:
    Scene& mScene;
    strutils::StringId mCurrentUpgradeNameUnlocked;
    std::vector<RepeatableFlow> mFlows;
    std::vector<strutils::StringId> mCreatedSceneObjectNames;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeUnlockedAnimationHandler_h */
