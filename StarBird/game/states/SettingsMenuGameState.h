///------------------------------------------------------------------------------------------------
///  SettingsMenuGameState.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SettingsMenuGameState_h
#define SettingsMenuGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class SettingsMenuGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void VInitialize() override;
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
    void VDestroy() override;

private:
    void UpdateSelectedSettingsColor();
    
private:
    std::vector<strutils::StringId> mSceneElementIds;
};

///------------------------------------------------------------------------------------------------

#endif /* SettingsMenuGameState_h */
