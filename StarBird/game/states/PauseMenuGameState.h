///------------------------------------------------------------------------------------------------
///  PauseMenuGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef PauseMenuGameState_h
#define PauseMenuGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class PauseMenuGameState final: public BaseGameState
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

#endif /* PauseMenuGameState_h */
