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
#include "../../utils/MathUtils.h"

#include <functional>
#include <unordered_map>
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
    struct CommandExecutionResult
    {
        CommandExecutionResult(bool success, const std::string& message)
            : mSuccess(success)
            , mMessage(message)
        {
        }
        
        const bool mSuccess;
        const std::string mMessage;
    };
    
private:
    static const glm::vec4 SUCCESS_COLOR;
    static const glm::vec4 FAILURE_COLOR;
    
private:
    void RegisterCommands();
    void ExecuteCommand(const std::string& command, const SceneObject& commandTextSo);
    void SetCommandExecutionOutput(const CommandExecutionResult& executionResult);
    void PostCommandExecution(const std::string& command, const SceneObject& commandTextSo);

private:
    std::unordered_map<strutils::StringId, std::function<CommandExecutionResult(const std::vector<std::string>&)>, strutils::StringIdHasher> mCommandMap;
    std::vector<strutils::StringId> mSceneElementIds;
    std::vector<strutils::StringId> mPastCommandElementIds;
};

///------------------------------------------------------------------------------------------------

#endif /* DebugConsoleGameState_h */
