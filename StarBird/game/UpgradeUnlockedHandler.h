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
#include "../utils/MathUtils.h"

#include "RepeatableFlow.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;
class b2World;

///------------------------------------------------------------------------------------------------

class UpgradeUnlockedHandler final
{
public:
    enum class UpgradeAnimationState
    {
        ONGOING, FINISHED
    };
    
    UpgradeUnlockedHandler(Scene& scene, b2World& box2dWorld);
    
    void OnUpgradeGained(const strutils::StringId& upgradeNameId);
    UpgradeAnimationState Update(const float dtMillis);

private:
    void OnCrystalGiftUpgradeGained();
    void OnHealthPotionUpgradeGained();
    void OnMirrorImageUpgradeGained();
    void OnDoubleBulletUpgradeGained();
    
    UpgradeAnimationState UpdateCrystalGiftUpgradeGained(const float dtMillis);
    UpgradeAnimationState UpdateHealthPotionUpgradeGained(const float dtMillis);
    UpgradeAnimationState UpdateMirrorImageUpgradeGained(const float dtMillis);
    UpgradeAnimationState UpdateDoubleBulletUpgradeGained(const float dtMillis);
    
private:
    Scene& mScene;
    b2World& mBox2dWorld;
    strutils::StringId mCurrentUpgradeNameUnlocked;
    std::vector<RepeatableFlow> mFlows;
    std::vector<strutils::StringId> mCreatedSceneObjectNames;
    bool mForceFinishAnimation;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeUnlockedHandler_h */
