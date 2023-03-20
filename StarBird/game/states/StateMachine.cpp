///------------------------------------------------------------------------------------------------
///  StateMachine.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "../../utils/OSMessageBox.h"
#include "BaseGameState.h"
#include "StateMachine.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId StateMachine::GetActiveStateName() const
{
    if (mStateStack.empty()) return strutils::StringId();
    
    for (const auto& entry: mStateNameToInstanceMap)
    {
        if (entry.second.get() == mStateStack.top())
        {
            return entry.first;
        }
    }
    
    return mStateNameToInstanceMap.begin()->first;
}

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
    if (mStateStack.empty()) return PostStateUpdateDirective::CONTINUE;
    
    while (mStateStack.top()->IsComplete())
    {
        if (mStateStack.top()->GetNextStateName() == BaseGameState::POP_STATE_COMPLETION_NAME)
        {
            mStateStack.top()->VDestroy();
            mStateStack.top()->mNextStateName = strutils::StringId();
            mStateStack.pop();
            
            if (mStateStack.size() == 0)
            {
                return PostStateUpdateDirective::CONTINUE;
            }
        }
        else
        {
            SwitchToState(mStateStack.top()->GetNextStateName());
        }
    }
    
    return mStateStack.top()->VUpdate(dtMillis);
}

///------------------------------------------------------------------------------------------------

void StateMachine::SwitchToState(const strutils::StringId& nextStateName, bool pushOnTop)
{
    auto iter = mStateNameToInstanceMap.find(nextStateName);
    if (iter != mStateNameToInstanceMap.end())
    {
        if (!mStateStack.empty() && !pushOnTop)
        {
            mStateStack.top()->VDestroy();
            mStateStack.top()->mNextStateName = strutils::StringId();
            mStateStack.pop();
        }
        
        mStateStack.push(iter->second.get());
        mStateStack.top()->VInitialize();
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Invalid State", "Invalid next state: " + nextStateName.GetString());
    }
}

///------------------------------------------------------------------------------------------------
