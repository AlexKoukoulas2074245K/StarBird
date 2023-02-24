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

class SceneObject;
class DebugConsoleGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void VInitialize() override;
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
    void VDestroy() override;

private:
    void ExecuteCommand(const std::string& command, const SceneObject& commandTextSo);
    void PostCommandExecution(const std::string& command, const SceneObject& commandTextSo);
    
private:
    std::vector<strutils::StringId> mSceneElementIds;
    std::vector<strutils::StringId> mPastCommandElementIds;
};

///------------------------------------------------------------------------------------------------

#endif /* DebugConsoleGameState_h */
