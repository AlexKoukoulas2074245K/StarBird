///------------------------------------------------------------------------------------------------
///  FightingWaveGameState.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef FightingWaveGameState_h
#define FightingWaveGameState_h

///------------------------------------------------------------------------------------------------

#include "BaseGameState.h"

///------------------------------------------------------------------------------------------------

class SceneObject;
class FightingWaveGameState final: public BaseGameState
{
public:
    static const strutils::StringId STATE_NAME;
    
public:
    void VInitialize() override;
    PostStateUpdateDirective VUpdate(const float dtMillis) override;
    
private:
    void UpdateExplodingSpecialEntity(const float dtMillis, SceneObject& sceneObject);
    
private:
    bool mBossDeathAnimationActive;
    bool mPlayerDeathAnimationActive;
};

///------------------------------------------------------------------------------------------------

#endif /* FightingWaveGameState_h */
