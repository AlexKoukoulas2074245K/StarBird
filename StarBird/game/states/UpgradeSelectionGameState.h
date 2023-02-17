///------------------------------------------------------------------------------------------------
///  UpgradeSelectionGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradeSelectionGameState_h
#define UpgradeSelectionGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class UpgradeSelectionGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void Initialize() override;
    PostStateUpdateDirective Update(const float dtMillis) override;
    void Destroy() override;
    
private:
    void CreateUpgradeSceneObjects();
    void UpdateOverlayIn(const float dtMillis);
    void UpdateUpgradeSelection(const float dtMillis);
    void UpdateShineSelection(const float dtMillis);
    void UpdateOverlayOut(const float dtMillis);
    
    strutils::StringId TestForUpgradeSelected();
    
private:
    enum class SubState
    {
        OVERLAY_IN,
        UPGRADE_SELECTION,
        SHINE_SELECTION,
        OVERLAY_OUT
    };
    
    enum class SelectionState
    {
        NONE,
        LEFT_SELECTED,
        RIGHT_SELECTED
    };
    
    SubState mState;
    SelectionState mSelectionState;
    float mAnimationTween = 0.0f;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeSelectionGameState_h */
