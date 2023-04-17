///------------------------------------------------------------------------------------------------
///  UpgradeUnlockedHandler.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradeUnlockedHandler_h
#define UpgradeUnlockedHandler_h

///------------------------------------------------------------------------------------------------

#include "../utils/StringUtils.h"
#include "RepeatableFlow.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;

///------------------------------------------------------------------------------------------------

class UpgradeUnlockedHandler final
{
public:
    enum class UpgradeAnimationState
    {
        ONGOING, FINISHED
    };
    
    UpgradeUnlockedHandler(Scene& scene);
    
    void OnUpgradeGained(const strutils::StringId& upgradeNameId);
    UpgradeAnimationState Update(const float dtMillis);

private:
    void OnCrystalGiftUpgradeGained();
    void OnHealthPotionUpgradeGained();
    
    UpgradeAnimationState UpdateCrystalGiftUpgradeGained();
    UpgradeAnimationState UpdateHealthPotionUpgradeGained();
    
private:
    Scene& mScene;
    strutils::StringId mCurrentUpgradeNameUnlocked;
    std::vector<RepeatableFlow> mFlows;
    std::vector<strutils::StringId> mCreatedSceneObjectNames;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeUnlockedHandler_h */
