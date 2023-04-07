///------------------------------------------------------------------------------------------------
///  ClearedLevelAnimationGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 07/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ClearedLevelAnimationGameState_h
#define ClearedLevelAnimationGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class ClearedLevelAnimationGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
};


///------------------------------------------------------------------------------------------------

#endif /* ClearedLevelAnimationGameState_h */
