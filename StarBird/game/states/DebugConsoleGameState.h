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
        CommandExecutionResult(bool success, const std::vector<std::string>& outputMessage)
            : mSuccess(success)
        {
            // Break down msesage further to wrap around edges of output area
            static int MAX_LINE_CHARS = 29;
            for (size_t i = 0; i < outputMessage.size(); ++i)
            {
                auto message = outputMessage[i];
                
                while (message.size() > MAX_LINE_CHARS)
                {
                    mOutputMessage.push_back(message.substr(0, MAX_LINE_CHARS) + "\\");
                    message = message.substr(MAX_LINE_CHARS);
                }
                
                mOutputMessage.push_back(message);
            }
        }
        
        CommandExecutionResult(bool success, const std::string& outputMessage)
            : CommandExecutionResult(success, std::vector<std::string>{outputMessage})
        {
        }
        
        const bool mSuccess;
        std::vector<std::string> mOutputMessage;
    };
    
private:
    static const glm::vec4 SUCCESS_COLOR;
    static const glm::vec4 FAILURE_COLOR;
    static const int SCROLL_LINE_THRESHOLD;
    static const float BIRDS_EYE_VIEW_CAMERA_LENSE_HEIGHT;
    static const float SCROLL_TOUCH_MIN_Y;
    static const float SCROLL_MIN_Y;
    static const float SCROLL_MAX_Y;
    
private:
    void RegisterCommands();
    void ExecuteCommand(const std::string& command, const SceneObject& commandTextSo);
    void SetCommandExecutionOutput(const CommandExecutionResult& executionResult);
    void PostCommandExecution(const std::string& command, const SceneObject& commandTextSo);

private:
    std::unordered_map<strutils::StringId, std::function<CommandExecutionResult(const std::vector<std::string>&)>, strutils::StringIdHasher> mCommandMap;
    std::vector<strutils::StringId> mSceneElementIds;
    std::vector<strutils::StringId> mPastCommandElementIds;
    std::vector<strutils::StringId> mCommandOutputElementIds;
    unsigned int mLastEventType = 0;
    int mPastCommandHistoryIndex = 0;
    float mPreviousCameraLenseHeight = 0.0f;
    float mPreviousMotionY = 0.0f;
};

///------------------------------------------------------------------------------------------------

#endif /* DebugConsoleGameState_h */
