///------------------------------------------------------------------------------------------------
///  GameConstants.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameConstants_h
#define GameConstants_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include <unordered_map>

///------------------------------------------------------------------------------------------------

namespace game_constants
{

///------------------------------------------------------------------------------------------------

inline const char* QUAD_MESH_FILE_NAME = "quad.obj";
inline const char* SMALL_CRYSTAL_MESH_FILE_NAME = "crystals/crystal_0.obj";

inline const char* BASIC_SHADER_FILE_NAME = "basic.vs";
inline const char* HUE_SHIFT_SHADER_FILE_NAME = "hue_shift.vs";
inline const char* DARKENED_COLOR_SHADER_FILE_NAME = "darkened_color.vs";
inline const char* DISSOLVE_SHADER_FILE_NAME = "dissolve.vs";
inline const char* SHINE_SHADER_FILE_NAME = "shine.vs";
inline const char* NEBULA_SHADER_FILE_NAME = "nebula.vs";
inline const char* PLAYER_SHIELD_SHADER_FILE_NAME = "player_shield.vs";
inline const char* BLACK_NEBULA_SHADER_FILE_NAME = "black_nebula.vs";
inline const char* TEXTURE_OFFSET_SHADER_FILE_NAME = "tex_offset.vs";
inline const char* CUSTOM_ALPHA_SHADER_FILE_NAME = "custom_alpha.vs";
inline const char* CUSTOM_COLOR_SHADER_FILE_NAME = "custom_color.vs";
inline const char* GRAYSCALE_SHADER_FILE_NAME = "grayscale.vs";
inline const char* DEBUG_CONSOLE_FONT_SHADER_FILE_NAME = "debug_console_font.vs";

inline const char* DEFAULT_BACKGROUND_TEXTURE_FILE_NAME = "backgrounds/bg/0.bmp";
inline const char* BACKGROUND_TEXTURE_FILE_PATH = "backgrounds/bg/";
inline const char* UPGRADE_SHINE_EFFECT_TEXTURE_FILE_NAME = "shine_effect_fxx.bmp";
inline const char* DISSOLVE_EFFECT_TEXTURE_FILE_NAME = "dissolve_line_fxy_mm.bmp";
inline const char* LAB_BACKGROUND_TEXTURE_FILE_NAME = "backgrounds/lab/lab_bg.bmp";
inline const char* CRYSTALS_TEXTURE_FILE_NAME = "crystal.bmp";
inline const char* CRYSTAL_HOLDER_TEXTURE_FILE_NAME = "crystal_count_holder.bmp";
inline const char* MIRROR_IMAGE_TEXTURE_FILE_NAME = "player_mirror_image_mm.bmp";
inline const char* BULLET_TEXTURE_FILE_NAME = "bullet.bmp";
inline const char* BETTER_BULLET_TEXTURE_FILE_NAME = "better_bullet.bmp";
inline const char* JOYSTICK_TEXTURE_FILE_NAME = "joystick.bmp";
inline const char* JOYSTICK_BOUNDS_TEXTURE_FILE_NAME = "joystick_bounds.bmp";
inline const char* UPGRADE_CONTAINER_TEXTURE_FILE_NAME = "upgrade_container.bmp";
inline const char* FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME = "overlay.bmp";
inline const char* PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME = "player_health_bar_frame.bmp";
inline const char* PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME = "player_health_bar.bmp";
inline const char* MAP_PLANET_TEXTURE_FILE_NAME = "planet_mm.bmp";
inline const char* MAP_PLANET_RING_TEXTURE_FILE_NAME = "planet_ring_mm.bmp";
inline const char* MAP_BASE_TEXTURE_FILE_NAME = "base_mm.bmp";
inline const char* MAP_STAR_PATH_TEXTURE_FILE_NAME = "star_path.bmp";
inline const char* NOISE_PREFIX_TEXTURE_FILE_NAME = "noise_";
inline const char* BOSS_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME = "player_health_bar_frame.bmp";
inline const char* BOSS_HEALTH_BAR_TEXTURE_FILE_NAME = "enemy_health_bar.bmp";
inline const char* HEALTH_PARTICLE_TEXTURE_FILE_NAME = "health_particle.bmp";

inline const strutils::StringId DEFAULT_SCENE_OBJECT_STATE = strutils::StringId("idle");
inline const strutils::StringId DYING_SCENE_OBJECT_STATE = strutils::StringId("dying");
inline const strutils::StringId DEFAULT_FONT_NAME = strutils::StringId("font");
inline const strutils::StringId DEFAULT_FONT_MM_NAME = strutils::StringId("font_mm");

inline const strutils::StringId WALL_SCENE_OBJECT_NAME = strutils::StringId("WALL");
inline const strutils::StringId PLAYER_SCENE_OBJECT_NAME = strutils::StringId("PLAYER");
inline const strutils::StringId PLAYER_SHIELD_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_SHIELD");
inline const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("BG");
inline const strutils::StringId NAVIGATION_ARROW_SCENE_OBJECT_NAME = strutils::StringId("NAVIGATION_ARROW");
inline const strutils::StringId JOYSTICK_SCENE_OBJECT_NAME = strutils::StringId("JOYSTICK");
inline const strutils::StringId JOYSTICK_BOUNDS_SCENE_OBJECT_NAME = strutils::StringId("JOYSTICK_BOUNDS");
inline const strutils::StringId WAVE_INTRO_TEXT_SCENE_OBJECT_NAME = strutils::StringId("WAVE_INTRO_TEXT");
inline const strutils::StringId BOSS_INTRO_TEXT_SCENE_OBJECT_NAME = strutils::StringId("BOSS_INTRO_TEXT");
inline const strutils::StringId LEFT_UPGRADE_SCENE_OBJECT_NAME = strutils::StringId("LEFT_UPGRADE");
inline const strutils::StringId LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("LEFT_UPGRADE_CONTAINER");
inline const strutils::StringId RIGHT_UPGRADE_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_UPGRADE");
inline const strutils::StringId RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_UPGRADE_CONTAINER");
inline const strutils::StringId FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("FULL_SCREEN_OVERLAY");
inline const strutils::StringId PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_BAR");
inline const strutils::StringId PLAYER_HEALTH_LOST_BAR_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_LOST_BAR");
inline const strutils::StringId PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_BAR_FRAME");
inline const strutils::StringId PLAYER_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_BAR_TEXT");
inline const strutils::StringId BOSS_HEALTH_BAR_SCENE_OBJECT_NAME = strutils::StringId("BOSS_HEALTH_BAR");
inline const strutils::StringId BOSS_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME = strutils::StringId("BOSS_HEALTH_BAR_FRAME");
inline const strutils::StringId BOSS_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME = strutils::StringId("BOSS_HEALTH_BAR_TEXT");
inline const strutils::StringId LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME = strutils::StringId("LEFT_MIRROR_IMAGE");
inline const strutils::StringId RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_MIRROR_IMAGE");
inline const strutils::StringId DEBUG_PAST_COMMAND_LINE_NAME_PREFIX = strutils::StringId("PAST_COMMAND_");
inline const strutils::StringId DEBUG_COMMAND_OUTPUT_LINE_NAME_PREFIX = strutils::StringId("OUTPUT_LINE_");
inline const strutils::StringId DEBUG_COMMAND_TEXT_SCENE_OBJECT_NAME = strutils::StringId("COMMAND_TEXT");
inline const strutils::StringId DEBUG_COMMAND_OUTPUT_SCENE_OBJECT_NAME = strutils::StringId("COMMAND_OUTPUT");
inline const strutils::StringId DEBUG_BACK_TO_GAME_SCENE_OBJECT_NAME = strutils::StringId("BACK_TO_GAME");
inline const strutils::StringId GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME = strutils::StringId("CRYSTAL_ICON");
inline const strutils::StringId GUI_CRYSTAL_COUNT_SCENE_OBJECT_NAME = strutils::StringId("CRYSTAL_COUNT");

inline const strutils::StringId TEXTURE_OFFSET_X_UNIFORM_NAME = strutils::StringId("tex_offset_x");
inline const strutils::StringId TEXTURE_OFFSET_Y_UNIFORM_NAME = strutils::StringId("tex_offset_y");
inline const strutils::StringId GENERIC_TEXTURE_OFFSET_UNIFORM_NAME = strutils::StringId("tex_offset");
inline const strutils::StringId SHINE_X_OFFSET_UNIFORM_NAME = strutils::StringId("shine_x_offset");
inline const strutils::StringId DISSOLVE_Y_OFFSET_UNIFORM_NAME = strutils::StringId("dissolve_y_offset");
inline const strutils::StringId IS_TEXTURE_SHEET_UNIFORM_NAME = strutils::StringId("texture_sheet");
inline const strutils::StringId IS_AFFECTED_BY_LIGHT_UNIFORM_NAME = strutils::StringId("affected_by_light");
inline const strutils::StringId MIN_U_UNIFORM_NAME = strutils::StringId("min_u");
inline const strutils::StringId MIN_V_UNIFORM_NAME = strutils::StringId("min_v");
inline const strutils::StringId MAX_U_UNIFORM_NAME = strutils::StringId("max_u");
inline const strutils::StringId MAX_V_UNIFORM_NAME = strutils::StringId("max_v");
inline const strutils::StringId CUSTOM_ALPHA_UNIFORM_NAME = strutils::StringId("custom_alpha");
inline const strutils::StringId CUSTOM_COLOR_UNIFORM_NAME = strutils::StringId("custom_color");
inline const strutils::StringId HUE_SHIFT_UNIFORM_NAME = strutils::StringId("hue_shift");
inline const strutils::StringId DARKEN_VALUE_UNIFORM_NAME = strutils::StringId("darken_value");

inline const strutils::StringId PLAYER_BULLET_FLOW_NAME = strutils::StringId("PLAYER_BULLET_FLOW");
inline const strutils::StringId PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME = strutils::StringId("PLAYER_INVINCIBILITY_FLOW");
inline const strutils::StringId WAVE_INTRO_FLOW_NAME = strutils::StringId("WAVE_INTRO_FLOW");
inline const strutils::StringId BOSS_INTRO_FLOW_NAME = strutils::StringId("BOSS_INTRO_FLOW");

inline const strutils::StringId PLAYER_OBJECT_TYPE_DEF_NAME = strutils::StringId("player");

inline const strutils::StringId DOUBLE_BULLET_UGPRADE_NAME = strutils::StringId("double_bullet");
inline const strutils::StringId MIRROR_IMAGE_UGPRADE_NAME = strutils::StringId("mirror_image");
inline const strutils::StringId PLAYER_SHIELD_UPGRADE_NAME = strutils::StringId("player_shield");
inline const strutils::StringId PLAYER_HEALTH_POTION_UGPRADE_NAME = strutils::StringId("player_health_potion");
inline const strutils::StringId CRYSTALS_GIFT_UGPRADE_NAME = strutils::StringId("crystal_gift");

inline const strutils::StringId PLAYER_BULLET_TYPE = strutils::StringId("player_bullet");
inline const strutils::StringId BETTER_PLAYER_BULLET_TYPE = strutils::StringId("player_better_bullet");

inline const strutils::StringId MIRROR_IMAGE_BULLET_TYPE = strutils::StringId("mirror_image_bullet");
inline const strutils::StringId BETTER_MIRROR_IMAGE_BULLET_TYPE = strutils::StringId("mirror_image_better_bullet");

inline const strutils::StringId TEXT_PROMPT_NAME = strutils::StringId("TEXT_PROMPT");

inline const strutils::StringId AMBIENT_LIGHT_NAME = strutils::StringId("AMBIENT_LIGHT");

inline const std::string ENEMY_PROJECTILE_FLOW_POSTFIX = std::string("_PROJECTILE_FLOW");

inline const float UPGRADE_MOVEMENT_SPEED = 1.0f/400.0f;

inline const float BOSS_INTRO_DURATION_MILLIS = 3000.0f;

inline const float BASE_PLAYER_BULLET_FLOW_DELAY_MILLIS = 300.0f;
inline const float PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS = 300.0f;

inline const float BASE_PLAYER_SPEED = 0.4f;
inline const float BACKGROUND_Z = -1.0f;
inline const float WALL_Z = -0.5f;
inline const float BULLET_Z = -0.5f;
inline const float PLAYER_BULLET_X_OFFSET = 0.48f;
inline const float MIRROR_IMAGE_BULLET_X_OFFSET = 0.38f;

inline const float GUI_CRYSTAL_ROTATION_SPEED = 0.0004f;

inline const float PLAYER_HEALTH_BAR_Z = 0.45f;
inline const float BOSS_HEALTH_BAR_Z = 2.45f;
inline const float HEALTH_LOST_SPEED = 0.0007f;
inline const float DROPPED_CRYSTALS_CREATION_STAGGER_MILLIS = 25.0f;
inline const float CRYSTAL_COUNT_CHANGE_SPEED = 0.07f;
inline const float BOSS_INTRO_ANIMATED_HEALTH_SPEED = 0.05f;
inline const float HEALTH_BAR_POSITION_DIVISOR_MAGIC = 2.15f;

inline const float FULL_SCREEN_OVERLAY_TRANSITION_DARKENING_SPEED = 1.0f/800.0f;
inline const float FULL_SCREEN_OVERLAY_TRANSITION_MAX_ALPHA = 1.0f;
inline const float FULL_SCREEN_OVERLAY_MENU_MAX_ALPHA = 0.8f;
inline const float FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED = 1.0f/400.0f;

inline const float SHINE_EFFECT_X_OFFSET_INIT_VAL = 1.0f;
inline const float SHINE_EFFECT_X_OFFSET_END_VAL = -1.0f;

inline const float DISSOLVE_EFFECT_Y_INIT_VAL = 1.0f;

inline const float NEBULA_ANIMATION_SPEED = 1.0f/15000.0f;

inline const float TEXT_FADE_IN_ALPHA_SPEED = 0.002f;

inline const float BACKGROUND_SPEED = 1.0f/4000.0f;

inline const glm::vec3 HEALTH_BAR_TEXT_SCALE = glm::vec3(0.006f, 0.006f, 1.0f);
inline const glm::vec3 HEALTH_BAR_TEXT_OFFSET = glm::vec3(0.0f, -0.2f, 0.5f);

inline const glm::vec3 PLAYER_HEALTH_BAR_POSITION = glm::vec3(0.0f, -12.0f, 0.5f);
inline const glm::vec3 PLAYER_HEALTH_BAR_SCALE = glm::vec3(5.0f, 1.2f, 1.0f);

inline const glm::vec3 PLAYER_INITIAL_POS = glm::vec3(0.0f, -10.0f, 0.0f);
inline const glm::vec3 PLAYER_CHEST_REWARD_POS = glm::vec3(0.0f, -7.0f, 0.0f);
inline const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);
inline const glm::vec3 MAP_BACKGROUND_SCALE = glm::vec3(120.0f, 120.0f, 1.0f);

inline const glm::vec3 BOSS_HEALTH_BAR_POSITION = glm::vec3(0.0f, 11.5f, 2.5f);
inline const glm::vec3 BOSS_HEALTH_BAR_SCALE = glm::vec3(10.0f, 1.0f, 1.0f);

inline const glm::vec4 AMBIENT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
inline const glm::vec4 POINT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

inline const glm::vec2 MAP_MAX_WORLD_BOUNDS = glm::vec2(40.0f, 20.0f);
inline const glm::vec2 MAP_MIN_WORLD_BOUNDS = glm::vec2(-17.0f, -13.0f);

inline const glm::vec3 GUI_CRYSTAL_POSITION = glm::vec3(-4.2f, -10.2f, 2.5f);
inline const glm::vec3 GUI_CRYSTAL_SCALE = glm::vec3(0.6f, 0.6f, 0.6f);

enum class LabOptionType
{
    REPAIR, STATS_UPGRADE, RESEARCH
};

inline const std::unordered_map<game_constants::LabOptionType, std::string> LAB_OPTION_TYPE_TO_TEXTURE =
{
    { LabOptionType::REPAIR, "backgrounds/lab/lab_option_repair.bmp" },
    { LabOptionType::STATS_UPGRADE, "backgrounds/lab/lab_option_crystal_transfer.bmp" },
    { LabOptionType::RESEARCH, "backgrounds/lab/lab_option_research.bmp" }
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* GameConstants_h */
