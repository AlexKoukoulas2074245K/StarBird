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

///------------------------------------------------------------------------------------------------

namespace game_constants
{

///------------------------------------------------------------------------------------------------

static const char* QUAD_MESH_FILE_NAME = "quad.obj";
static const char* MAP_PLANET_MESH_FILE_NAME = "planet.obj";
static const char* MAP_PLANET_RING_MESH_FILE_NAME = "planet_ring.obj";
static const char* MAP_BASE_MESH_FILE_NAME = "base.obj";
static const char* BASIC_SHADER_FILE_NAME = "basic.vs";
static const char* HUE_SHIFT_SHADER_FILE_NAME = "hue_shift.vs";
static const char* DISSOLVE_SHADER_FILE_NAME = "dissolve.vs";
static const char* SHINE_SHADER_FILE_NAME = "shine.vs";
static const char* NEBULA_SHADER_FILE_NAME = "nebula.vs";
static const char* BLACK_NEBULA_SHADER_FILE_NAME = "black_nebula.vs";
static const char* TEXTURE_OFFSET_SHADER_FILE_NAME = "tex_offset.vs";
static const char* CUSTOM_ALPHA_SHADER_FILE_NAME = "custom_alpha.vs";
static const char* CUSTOM_COLOR_SHADER_FILE_NAME = "custom_color.vs";
static const char* DEBUG_CONSOLE_FONT_SHADER_FILE_NAME = "debug_console_font.vs";
static const char* UPGRADE_SHINE_EFFECT_TEXTURE_FILE_NAME = "shine_effect_fxx.bmp";
static const char* DISSOLVE_EFFECT_TEXTURE_FILE_NAME = "dissolve_line_fxy_mm.bmp";
static const char* BACKGROUND_TEXTURE_FILE_NAME = "backgrounds/green/3.bmp";
static const char* MIRROR_IMAGE_TEXTURE_FILE_NAME = "player_mirror_image_mm.bmp";
static const char* BULLET_TEXTURE_FILE_NAME = "bullet.bmp";
static const char* BETTER_BULLET_TEXTURE_FILE_NAME = "better_bullet.bmp";
static const char* JOYSTICK_TEXTURE_FILE_NAME = "joystick.bmp";
static const char* JOYSTICK_BOUNDS_TEXTURE_FILE_NAME = "joystick_bounds.bmp";
static const char* UPGRADE_CONTAINER_TEXTURE_FILE_NAME = "upgrade_container.bmp";
static const char* FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME = "overlay.bmp";
static const char* PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME = "player_health_bar_frame_mm.bmp";
static const char* PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME = "player_health_bar.bmp";
static const char* MAP_PLANET_TEXTURE_FILE_NAME = "planet_mm.bmp";
static const char* MAP_PLANET_RING_TEXTURE_FILE_NAME = "planet_ring_mm.bmp";
static const char* MAP_BASE_TEXTURE_FILE_NAME = "base_mm.bmp";
static const char* MAP_STAR_PATH_TEXTURE_FILE_NAME = "star_path.bmp";
static const char* NOISE_PREFIX_TEXTURE_FILE_NAME = "noise_";
static const char* BOSS_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME = "player_health_bar_frame_mm.bmp";
static const char* BOSS_HEALTH_BAR_TEXTURE_FILE_NAME = "enemy_health_bar.bmp";
static const char* PLAYER_HEALTH_LOST_BAR_TEXTURE_FILE_NAME = "player_health_lost_bar.bmp";
static const char* PLAYER_SHIELD_TEXTURE_FILE_NAME = "player_shield.bmp";

static const strutils::StringId DEFAULT_SCENE_OBJECT_STATE = strutils::StringId("idle");
static const strutils::StringId DEFAULT_FONT_NAME = strutils::StringId("font");

static const strutils::StringId WALL_SCENE_OBJECT_NAME = strutils::StringId("WALL");
static const strutils::StringId PLAYER_SCENE_OBJECT_NAME = strutils::StringId("PLAYER");
static const strutils::StringId PLAYER_SHIELD_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_SHIELD");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("BG");
static const strutils::StringId JOYSTICK_SCENE_OBJECT_NAME = strutils::StringId("JOYSTICK");
static const strutils::StringId JOYSTICK_BOUNDS_SCENE_OBJECT_NAME = strutils::StringId("JOYSTICK_BOUNDS");
static const strutils::StringId WAVE_INTRO_TEXT_SCENE_OBJECT_NAME = strutils::StringId("WAVE_INTRO_TEXT");
static const strutils::StringId BOSS_INTRO_TEXT_SCENE_OBJECT_NAME = strutils::StringId("BOSS_INTRO_TEXT");
static const strutils::StringId LEFT_UPGRADE_SCENE_OBJECT_NAME = strutils::StringId("LEFT_UPGRADE");
static const strutils::StringId LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("LEFT_UPGRADE_CONTAINER");
static const strutils::StringId RIGHT_UPGRADE_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_UPGRADE");
static const strutils::StringId RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_UPGRADE_CONTAINER");
static const strutils::StringId FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("FULL_SCREEN_OVERLAY");
static const strutils::StringId PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_BAR");
static const strutils::StringId PLAYER_HEALTH_LOST_BAR_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_LOST_BAR");
static const strutils::StringId PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_HEALTH_BAR_FRAME");
static const strutils::StringId BOSS_HEALTH_BAR_SCENE_OBJECT_NAME = strutils::StringId("BOSS_HEALTH_BAR");
static const strutils::StringId BOSS_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME = strutils::StringId("BOSS_HEALTH_BAR_FRAME");
static const strutils::StringId LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME = strutils::StringId("LEFT_MIRROR_IMAGE");
static const strutils::StringId RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_MIRROR_IMAGE");
static const strutils::StringId DEBUG_PAST_COMMAND_LINE_NAME_PREFIX = strutils::StringId("PAST_COMMAND_");
static const strutils::StringId DEBUG_COMMAND_OUTPUT_LINE_NAME_PREFIX = strutils::StringId("OUTPUT_LINE_");
static const strutils::StringId DEBUG_COMMAND_TEXT_SCENE_OBJECT_NAME = strutils::StringId("COMMAND_TEXT");
static const strutils::StringId DEBUG_COMMAND_OUTPUT_SCENE_OBJECT_NAME = strutils::StringId("COMMAND_OUTPUT");
static const strutils::StringId DEBUG_BACK_TO_GAME_SCENE_OBJECT_NAME = strutils::StringId("BACK_TO_GAME");

static const strutils::StringId TEXTURE_OFFSET_X_UNIFORM_NAME = strutils::StringId("tex_offset_x");
static const strutils::StringId TEXTURE_OFFSET_Y_UNIFORM_NAME = strutils::StringId("tex_offset_y");
static const strutils::StringId GENERIC_TEXTURE_OFFSET_UNIFORM_NAME = strutils::StringId("tex_offset");
static const strutils::StringId SHINE_X_OFFSET_UNIFORM_NAME = strutils::StringId("shine_x_offset");
static const strutils::StringId DISSOLVE_Y_OFFSET_UNIFORM_NAME = strutils::StringId("dissolve_y_offset");
static const strutils::StringId IS_TEXTURE_SHEET_UNIFORM_NAME = strutils::StringId("texture_sheet");
static const strutils::StringId IS_AFFECTED_BY_LIGHT_UNIFORM_NAME = strutils::StringId("affected_by_light");
static const strutils::StringId MIN_U_UNIFORM_NAME = strutils::StringId("min_u");
static const strutils::StringId MIN_V_UNIFORM_NAME = strutils::StringId("min_v");
static const strutils::StringId MAX_U_UNIFORM_NAME = strutils::StringId("max_u");
static const strutils::StringId MAX_V_UNIFORM_NAME = strutils::StringId("max_v");
static const strutils::StringId CUSTOM_ALPHA_UNIFORM_NAME = strutils::StringId("custom_alpha");
static const strutils::StringId CUSTOM_COLOR_UNIFORM_NAME = strutils::StringId("custom_color");
static const strutils::StringId HUE_SHIFT_UNIFORM_NAME = strutils::StringId("hue_shift");

static const strutils::StringId PLAYER_BULLET_FLOW_NAME = strutils::StringId("PLAYER_BULLET_FLOW");
static const strutils::StringId PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME = strutils::StringId("PLAYER_INVINCIBILITY_FLOW");
static const strutils::StringId WAVE_INTRO_FLOW_NAME = strutils::StringId("WAVE_INTRO_FLOW");
static const strutils::StringId BOSS_INTRO_FLOW_NAME = strutils::StringId("BOSS_INTRO_FLOW");

static const strutils::StringId PLAYER_OBJECT_TYPE_DEF_NAME = strutils::StringId("player");

static const strutils::StringId BULLET_DAMAGE_UGPRADE_NAME = strutils::StringId("bullet_dmg");
static const strutils::StringId DOUBLE_BULLET_UGPRADE_NAME = strutils::StringId("double_bullet");
static const strutils::StringId MIRROR_IMAGE_UGPRADE_NAME = strutils::StringId("mirror_image");
static const strutils::StringId BULLET_SPEED_UGPRADE_NAME = strutils::StringId("bullet_speed");
static const strutils::StringId PLAYER_SPEED_UGPRADE_NAME = strutils::StringId("player_speed");
static const strutils::StringId PLAYER_SHIELD_UPGRADE_NAME = strutils::StringId("player_shield");
static const strutils::StringId PLAYER_HEALTH_POTION_UGPRADE_NAME = strutils::StringId("player_health_potion");

static const strutils::StringId PLAYER_BULLET_TYPE = strutils::StringId("player_bullet");
static const strutils::StringId BETTER_PLAYER_BULLET_TYPE = strutils::StringId("player_better_bullet");

static const strutils::StringId MIRROR_IMAGE_BULLET_TYPE = strutils::StringId("mirror_image_bullet");
static const strutils::StringId BETTER_MIRROR_IMAGE_BULLET_TYPE = strutils::StringId("mirror_image_better_bullet");

static const std::string ENEMY_PROJECTILE_FLOW_POSTFIX = std::string("_PROJECTILE_FLOW");

static const float BACKGROUND_SPEED = 1.0f/4000.0f;

static const float WAVE_INTRO_TEXT_SPEED = 0.01f;
static const float BOSS_INTRO_TEXT_SPEED = 0.01f;

static const float UPGRADE_MOVEMENT_SPEED = 1.0f/400.0f;

static const float WAVE_INTRO_DURATION_MILLIS = 3000.0f;
static const float BOSS_INTRO_DURATION_MILLIS = 3000.0f;

static const float PLAYER_BULLET_FLOW_DELAY_MILLIS = 300.0f;
static const float HASTENED_PLAYER_BULLET_FLOW_DELAY_MILLIS = 200.0f;

static const float PLAYER_BODY_X_SCALE = 0.5f;
static const float PLAYER_SPEED_UPGRADE_MULTIPLIER = 1.5f;

static const float PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS = 300.0f;

static const float JOYSTICK_Z = 1.0f;
static const float JOYSTICK_BOUNDS_Z = 2.0f;

static const float BACKGROUND_Z = -1.0f;
static const float WALL_Z = -0.5f;

static const float BULLET_Z = -0.5f;

static const float PLAYER_HEALTH_LOST_BAR_Z = 0.4f;
static const float PLAYER_HEALTH_BAR_Z = 0.45f;
static const float BOSS_HEALTH_BAR_Z = 2.45f;
static const float HEALTH_LOST_SPEED = 0.001f;
static const float BOSS_INTRO_ANIMATED_HEALTH_SPEED = 0.05f;
static const float HEALTH_BAR_POSITION_DIVISOR_MAGIC = 2.15f;

static const float FULL_SCREEN_OVERLAY_TRANSITION_DARKENING_SPEED = 1.0f/800.0f;
static const float FULL_SCREEN_OVERLAY_TRANSITION_MAX_ALPHA = 1.0f;
static const float FULL_SCREEN_OVERLAY_MENU_MAX_ALPHA = 0.8f;
static const float FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED = 1.0f/400.0f;

static const float PLAYER_BULLET_X_OFFSET = 0.48f;

static const float MIRROR_IMAGE_BULLET_X_OFFSET = 0.38f;

static const float HEALTH_POTION_HEALTH_GAIN = 5.0f;

static const float PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR = 1.0f/50.0f;
static const float PLAYER_PULSE_SHIELD_ANIM_SPEED = 0.01f;

static const float SELECTED_UPGRADE_PULSE_ENLARGEMENT_FACTOR = 1.0f/5.0f;
static const float SELECTED_UPGRADE_PULSE_ANIM_SPEED = 0.03f;

static const float PLAYER_MOVEMENT_ROLL_CHANCE = 0.333f;
static const float PLAYER_MOVEMENT_ROLL_SPEED = 0.008f;
static const float PLAYER_MOVEMENT_ROLL_ANGLE = 180.0f;

static const float EXPLOSION_LIGHT_POWER = 1.0f;
static const float EXPLOSION_LIGHT_FADE_SPEED = 1.0f/400.0f;

static const float MAP_BASE_X_ROTATION = 0.6f;
static const float MAP_STAR_PATH_PULSING_DELAY_MILLIS = 100.0f;
static const float MAP_STAR_PATH_PULSING_SPEED = 0.01f;
static const float MAP_STAR_PATH_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;
static const float MAP_PLANET_RING_MIN_X_ROTATION = 1.8f;
static const float MAP_PLANET_RING_MAX_X_ROTATION = 2.2f;
static const float MAP_PLANET_RING_MIN_Y_ROTATION = -math::PI/10;
static const float MAP_PLANET_RING_MAX_Y_ROTATION = +math::PI/10;

static const float SHINE_EFFECT_X_OFFSET_INIT_VAL = 1.0f;
static const float SHINE_EFFECT_X_OFFSET_END_VAL = -1.0f;
static const float DEBUG_PAST_COMMAND_X_OFFSET = -1.0f;
static const float DEBUG_PAST_COMMAND_Y_OFFSET = 1.0f;
static const float UPGRADE_SHINE_EFFECT_SPEED = 1.0f/200.0f;
static const float PLAYER_SHINE_EFFECT_SPEED = 1.0f/400.0f;

static const float DISSOLVE_EFFECT_Y_INIT_VAL = 1.0f;
static const float DISSOLVE_EFFECT_SPEED = 1.0f/1000.0f;

static const float NEBULA_ANIMATION_SPEED = 1.0f/15000.0f;

static const glm::vec3 PLAYER_INITIAL_POS = glm::vec3(0.0f, -10.0f, 0.0f);
static const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);
static const glm::vec3 MAP_BACKGROUND_SCALE = glm::vec3(120.0f, 120.0f, 1.0f);

static const glm::vec3 JOYSTICK_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);
static const glm::vec3 JOYSTICK_BOUNDS_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);

static const glm::vec3 WAVE_INTRO_TEXT_INIT_POS = glm::vec3(-3.0f, 0.0f, 2.0f);
static const glm::vec3 WAVE_INTRO_TEXT_SCALE = glm::vec3(0.02f, 0.02f, 1.0f);

static const glm::vec3 BOSS_INTRO_TEXT_INIT_POS = glm::vec3(-3.0f, 0.0f, 2.0f);
static const glm::vec3 BOSS_INTRO_TEXT_SCALE = glm::vec3(0.02f, 0.02f, 1.0f);

static const glm::vec3 LEFT_UPGRADE_INIT_POS = glm::vec3(-11.0f, 0.5f, 5.0f);
static const glm::vec3 LEFT_UPGRADE_TARGET_POS = glm::vec3(-2.0f, 0.5f, 5.0f);
static const glm::vec3 LEFT_UPGRADE_SCALE = glm::vec3(2.5f, 2.5f, 1.0f);

static const glm::vec3 LEFT_UPGRADE_CONTAINER_INIT_POS = glm::vec3(-11.0f, 0.0f, 5.0f);
static const glm::vec3 LEFT_UPGRADE_CONTAINER_TARGET_POS = glm::vec3(-4.0f, 0.0f, 5.0f);
static const glm::vec3 LEFT_UPGRADE_CONTAINER_SCALE = glm::vec3(8.5f, 5.5f, 1.0f);

static const glm::vec3 RIGHT_UPGRADE_INIT_POS = glm::vec3(11.0f, 0.5f, 5.0f);
static const glm::vec3 RIGHT_UPGRADE_TARGET_POS = glm::vec3(2.0f, 0.5f, 5.0f);
static const glm::vec3 RIGHT_UPGRADE_SCALE = glm::vec3(2.5f, 2.5f, 1.0f);

static const glm::vec3 RIGHT_UPGRADE_CONTAINER_INIT_POS = glm::vec3(11.0f, 0.0f, 5.0f);
static const glm::vec3 RIGHT_UPGRADE_CONTAINER_TARGET_POS = glm::vec3(4.0f, 0.0f, 5.0f);
static const glm::vec3 RIGHT_UPGRADE_CONTAINER_SCALE = glm::vec3(-8.5f, 5.5f, 1.0f);

static const glm::vec3 FULL_SCREEN_OVERLAY_POSITION = glm::vec3(0.0f, 0.0f, 3.0f);
static const glm::vec3 FULL_SCREEN_OVERLAY_SCALE = glm::vec3(200.0f, 200.0f, 1.0f);

static const glm::vec3 PLAYER_HEALTH_BAR_POSITION = glm::vec3(0.0f, -12.0f, 0.5f);
static const glm::vec3 PLAYER_HEALTH_BAR_SCALE = glm::vec3(5.0f, 1.0f, 1.0f);

static const glm::vec3 BOSS_HEALTH_BAR_POSITION = glm::vec3(0.0f, 11.5f, 2.5f);
static const glm::vec3 BOSS_HEALTH_BAR_SCALE = glm::vec3(10.0f, 1.0f, 1.0f);

static const glm::vec3 LEFT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(-2.0f, -0.5f, 0.0f);
static const glm::vec3 LEFT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 RIGHT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(2.0f, -0.5f, 0.0f);
static const glm::vec3 RIGHT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 PLAYER_SHIELD_POSITION_OFFSET = glm::vec3(0.0f, 0.5f, 0.5f);
static const glm::vec3 PLAYER_SHIELD_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);

static const glm::vec4 AMBIENT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
static const glm::vec4 POINT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

static const glm::vec2 MAP_MAX_WORLD_BOUNDS = glm::vec2(40.0f, 20.0f);
static const glm::vec2 MAP_MIN_WORLD_BOUNDS = glm::vec2(-17.0f, -13.0f);

static const glm::vec3 MAP_NEBULA_NODE_SCALE = glm::vec3(3.0f, 3.0f, 1.0f);
static const glm::vec3 MAP_STAR_PATH_SCALE = glm::vec3(0.3f, 0.3f, 1.0f);
static const glm::vec3 MAP_BASE_SCALE = glm::vec3(0.9, 0.5f, 0.9f);

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* GameConstants_h */
