///------------------------------------------------------------------------------------------------
///  UpgradeOverlayOutGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradeOverlayOutGameState_h
#define UpgradeOverlayOutGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class UpgradeOverlayOutGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void Initialize() override;
    PostStateUpdateDirective Update(const float dtMillis) override;
    void Destroy() override;
    
private:
    float mAnimationTween;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeOverlayOutGameState_h */
