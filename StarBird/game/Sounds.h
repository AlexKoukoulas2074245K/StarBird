///------------------------------------------------------------------------------------------------
///  Sounds.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/05/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Sounds_h
#define Sounds_h

///------------------------------------------------------------------------------------------------

#include <string>

///------------------------------------------------------------------------------------------------

namespace sounds
{
    inline const std::string MENU_THEME = "piano_theme";
    inline const std::string BATTLE_THEME = "ambience";
    inline const std::string BOSS_THEME = "boss_theme";
    inline const std::string CHEST_REWARD_THEME = "heartbeat";

    inline const std::string BULLET_SFX = "sfx_bullet";
    inline const std::string ENEMY_EXPLOSION_SFX = "sfx_impact_1";
    inline const std::string PLAYER_DAMAGED_SFX = "sfx_impact_2";
    inline const std::string PLAYER_BOSS_EXPLOSION_SFX = "sfx_boss_explosion";
    inline const std::string TRANSLOCATION_SFX = "sfx_translocation";
    inline const std::string WHOOSH_SFX = "sfx_whoosh";
    inline const std::string CRYSTALS_SFX = "sfx_crystals";
    inline const std::string BOSS_INTRO_SFX = "sfx_boss_intro";
    inline const std::string BOSS_SCREAM_SFX = "sfx_boss_scream";
    inline const std::string SWIPE_SFX = "sfx_swipe";
    inline const std::string CHEST_OPEN_SFX = "sfx_success";
    inline const std::string SUCCESS_CHIME_SFX = "sfx_success_chime";
    inline const std::string HEALTH_SFX = "sfx_health";
    inline const std::string BUTTON_PRESS_SFX = "sfx_button_press";
    inline const std::string BARRIER_SFX = "sfx_barrier";
}
///------------------------------------------------------------------------------------------------

#endif /* Sounds_h */
