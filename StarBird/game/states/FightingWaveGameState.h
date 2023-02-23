///------------------------------------------------------------------------------------------------
///  FightingWaveGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef FightingWaveGameState_h
#define FightingWaveGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class FightingWaveGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
    FightingWaveGameState();
    
public:
    void VInitialize() override;
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
    void UpdateHealthBars(const float dtMillis);
    
private:
    float mAnimatedHealthBarPerc;
};

///------------------------------------------------------------------------------------------------

#endif /* FightingWaveGameState_h */
