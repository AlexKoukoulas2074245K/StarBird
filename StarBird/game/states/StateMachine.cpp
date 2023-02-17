///------------------------------------------------------------------------------------------------
///  StateMachine.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "../../utils/OSMessageBox.h"
#include "StateMachine.h"

///------------------------------------------------------------------------------------------------

void StateMachine::InitStateMachine(const strutils::StringId& initStateName)
{
    SwitchToState(initStateName);
}

///------------------------------------------------------------------------------------------------

void StateMachine::PushState(const strutils::StringId& stateName)
{
    SwitchToState(stateName, true);
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective StateMachine::Update(const float dtMillis)
{
    while (mStateStack.top()->IsComplete())
    {
        if (mStateStack.top()->GetNextStateName() == BaseGameState::POP_STATE_COMPLETION_NAME)
        {
            mStateStack.top()->Destroy();
            mStateStack.top()->mNextStateName = strutils::StringId();
            mStateStack.pop();
        }
        else
        {
            SwitchToState(mStateStack.top()->GetNextStateName());
        }
    }
    
    return mStateStack.top()->Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

void StateMachine::SwitchToState(const strutils::StringId& nextStateName, bool pushOnTop)
{
    auto iter = mStateNameToInstanceMap.find(nextStateName);
    if (iter != mStateNameToInstanceMap.end())
    {
        if (!mStateStack.empty() && !pushOnTop)
        {
            mStateStack.top()->Destroy();
            mStateStack.top()->mNextStateName = strutils::StringId();
            mStateStack.pop();
        }
        
        mStateStack.push(iter->second.get());
        mStateStack.top()->Initialize();
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Invalid State", "Invalid next state: " + nextStateName.GetString());
    }
}

///------------------------------------------------------------------------------------------------
