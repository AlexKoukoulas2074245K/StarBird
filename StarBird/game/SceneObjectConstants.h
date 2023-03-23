///------------------------------------------------------------------------------------------------
///  SceneObjectConstants.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneObjectConstants_h
#define SceneObjectConstants_h

///------------------------------------------------------------------------------------------------

#include "../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

namespace scene_object_constants
{

///------------------------------------------------------------------------------------------------

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

static const float SHINE_EFFECT_X_OFFSET_INIT_VAL = 1.0f;
static const float SHINE_EFFECT_X_OFFSET_END_VAL = -1.0f;
static const float DEBUG_PAST_COMMAND_X_OFFSET = -1.0f;
static const float DEBUG_PAST_COMMAND_Y_OFFSET = 1.0f;
static const float UPGRADE_SHINE_EFFECT_SPEED = 1.0f/200.0f;
static const float PLAYER_SHINE_EFFECT_SPEED = 1.0f/400.0f;

static const float DISSOLVE_EFFECT_Y_INIT_VAL = 1.0f;
static const float DISSOLVE_EFFECT_SPEED = 1.0f/1000.0f;

static const float NEBULA_ANIMATION_SPEED = 1.0f/15000.0f;

static const char* QUAD_MESH_FILE_NAME = "quad.obj";
static const char* MAP_PLANET_MESH_FILE_NAME = "planet.obj";
static const char* MAP_PLANET_RING_MESH_FILE_NAME = "planet_ring.obj";
static const char* BASIC_SHADER_FILE_NAME = "basic.vs";
static const char* HUE_SHIFT_SHADER_FILE_NAME = "hue_shift.vs";
static const char* DISSOLVE_SHADER_FILE_NAME = "dissolve.vs";
static const char* SHINE_SHADER_FILE_NAME = "shine.vs";
static const char* NEBULA_SHADER_FILE_NAME = "nebula.vs";
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
static const char* MAP_PLANET_TEXTURE_FILE_NAME = "planet.bmp";
static const char* MAP_PLANET_RING_TEXTURE_FILE_NAME = "planet_ring.bmp";
static const char* NOISE_0_TEXTURE_FILE_NAME = "noise_0.bmp";
static const char* NOISE_1_TEXTURE_FILE_NAME = "noise_1.bmp";
static const char* BOSS_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME = "player_health_bar_frame_mm.bmp";
static const char* BOSS_HEALTH_BAR_TEXTURE_FILE_NAME = "enemy_health_bar.bmp";
static const char* PLAYER_HEALTH_LOST_BAR_TEXTURE_FILE_NAME = "player_health_lost_bar.bmp";
static const char* PLAYER_SHIELD_TEXTURE_FILE_NAME = "player_shield.bmp";

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObjectConstants_h */
