///------------------------------------------------------------------------------------------------
///  UpgradeOverlayInGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradeOverlayInGameState_h
#define UpgradeOverlayInGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class UpgradeOverlayInGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void Initialize() override;
    PostStateUpdateDirective Update(const float dtMillis) override;
    
private:
    void CreateUpgradeSceneObjects();
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeOverlayInGameState_h */
