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

PostStateUpdateDirective StateMachine::Update(const float dtMillis)
{
    while (mCurrentState->IsComplete())
    {
        SwitchToState(mCurrentState->GetNextStateName());
    }
    
    return mCurrentState->Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

void StateMachine::SwitchToState(const strutils::StringId& nextStateName)
{
    auto iter = mStateNameToInstanceMap.find(nextStateName);
    if (iter != mStateNameToInstanceMap.end())
    {
        if (mCurrentState)
        {
            mCurrentState->Destroy();
            mCurrentState->mNextStateName = strutils::StringId();
        }
        
        mCurrentState = iter->second.get();
        mCurrentState->Initialize();
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Invalid State", "Invalid next state: " + mCurrentState->GetNextStateName().GetString());
    }
}

///------------------------------------------------------------------------------------------------
