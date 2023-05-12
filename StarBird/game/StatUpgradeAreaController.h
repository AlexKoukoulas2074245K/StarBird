///------------------------------------------------------------------------------------------------
///  StatUpgradeAreaController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef StatUpgradeAreaController_h
#define StatUpgradeAreaController_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <string>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class BaseAnimation;
class Scene;
class SceneObject;
class StatUpgradeAreaController final
{
public:
    StatUpgradeAreaController(Scene& scene, std::unique_ptr<BaseAnimation> statUpgradeBackgroundAnimation, const glm::vec3& position, const glm::vec3& additionalOffsetForContainedSceneObjects, const glm::vec3& scale, const std::string& text, const float defaultStatValue, const float initialStatValue, const float statIncrement, const bool floatDisplay);
    
    float GetCurrentStatValue() const;
    float GetCurrentCost() const;
    const glm::vec3& GetTargetCrystalPosition() const;
    
    void HideMissingCrystals();
    void Update(const float dtMillis, const float currentTotalCost);
    
private:
    int CalculateStatCost(const float statValue);

private:
    Scene& mScene;
    const float mStatIncrement;
    const float mDefaultStatValue;
    const float mInitialStatValue;
    const bool mFloatDisplay;
    glm::vec3 mTargetCrystalPosition;
    float mStatValue;
    int mCurrentCost;
    Uint32 mLastInputContextEventType;
    strutils::StringId mPlusButtonName;
    strutils::StringId mMissingCrystalsName;
    strutils::StringId mMissingCrystalsIconName;
    strutils::StringId mMinusButtonName;
    strutils::StringId mStatValueTextName;
    strutils::StringId mUpgradeCostTextName;
    bool mMissingCrystalsSceneObjectsDisplayDisabled;
};

///------------------------------------------------------------------------------------------------

#endif /* StatUpgradeAreaController_h */
