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
 
private:
    strutils::StringId TestForUpgradeSelected();
    
private:
    float mAnimationTween = 0.0f;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeSelectionGameState_h */
