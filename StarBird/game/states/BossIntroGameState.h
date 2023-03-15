///------------------------------------------------------------------------------------------------
///  BossIntroGameState.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/03/2023
///------------------------------------------------------------------------------------------------

#ifndef BossIntroGameState_h
#define BossIntroGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class BossIntroGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void VInitialize() override;
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
    void VDestroy() override;
    
private:
    enum class SubState
    {
        BOSS_NAME_DISPLAY,
        BOSS_HEALTH_BAR_ANIMATION
    };
    
    SubState mSubState;
};

///------------------------------------------------------------------------------------------------

#endif /* BossIntroGameState_h */
