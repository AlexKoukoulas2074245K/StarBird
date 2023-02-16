///------------------------------------------------------------------------------------------------
///  WaveIntroGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef WaveIntroGameState_h
#define WaveIntroGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class WaveIntroGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void Initialize() override;
    PostStateUpdateDirective Update(const float dtMillis) override;
    void Destroy() override;
};

///------------------------------------------------------------------------------------------------

#endif /* WaveIntroGameState_h */
