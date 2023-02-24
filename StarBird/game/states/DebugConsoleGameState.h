///------------------------------------------------------------------------------------------------
///  DebugConsoleGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef DebugConsoleGameState_h
#define DebugConsoleGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class DebugConsoleGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void VInitialize() override;
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
    void VDestroy() override;

private:
    std::vector<strutils::StringId> mSceneElementIds;
};

///------------------------------------------------------------------------------------------------

#endif /* DebugConsoleGameState_h */
