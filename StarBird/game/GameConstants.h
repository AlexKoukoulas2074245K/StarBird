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

static const char* QUAD_MESH_FILE_NAME = "quad.obj";
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
static const char* BACKGROUND_TEXTURE_FILE_NAME = "backgrounds/blue/0.bmp";
static const char* LAB_BACKGROUND_TEXTURE_FILE_NAME = "backgrounds/lab/lab_bg.bmp";
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

static const strutils::StringId DEFAULT_SCENE_OBJECT_STATE = strutils::StringId("idle");
static const strutils::StringId DEFAULT_FONT_NAME = strutils::StringId("font_mm");

static const strutils::StringId WALL_SCENE_OBJECT_NAME = strutils::StringId("WALL");
static const strutils::StringId PLAYER_SCENE_OBJECT_NAME = strutils::StringId("PLAYER");
static const strutils::StringId PLAYER_SHIELD_SCENE_OBJECT_NAME = strutils::StringId("PLAYER_SHIELD");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("BG");
static const strutils::StringId NAVIGATION_ARROW_SCENE_OBJECT_NAME = strutils::StringId("NAVIGATION_ARROW");
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
static const strutils::StringId LAB_OPTION_NAME_PREFIX = strutils::StringId("LAB_OPTION_");
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

static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT");

static const strutils::StringId TEXT_PROMPT_NAME = strutils::StringId("TEXT_PROMPT");

static const std::string ENEMY_PROJECTILE_FLOW_POSTFIX = std::string("_PROJECTILE_FLOW");

static const float UPGRADE_MOVEMENT_SPEED = 1.0f/400.0f;

static const float BOSS_INTRO_DURATION_MILLIS = 3000.0f;

static const float PLAYER_BULLET_FLOW_DELAY_MILLIS = 300.0f;
static const float HASTENED_PLAYER_BULLET_FLOW_DELAY_MILLIS = 200.0f;
static const float PLAYER_INVINCIBILITY_FLOW_DELAY_MILLIS = 300.0f;

static const float BACKGROUND_Z = -1.0f;
static const float LAB_OPTIONS_Z = 2.0f;
static const float WALL_Z = -0.5f;
static const float BULLET_Z = -0.5f;

static const float PLAYER_HEALTH_BAR_Z = 0.45f;
static const float BOSS_HEALTH_BAR_Z = 2.45f;
static const float HEALTH_LOST_SPEED = 0.001f;
static const float BOSS_INTRO_ANIMATED_HEALTH_SPEED = 0.05f;

static const float FULL_SCREEN_OVERLAY_TRANSITION_DARKENING_SPEED = 1.0f/800.0f;
static const float FULL_SCREEN_OVERLAY_TRANSITION_MAX_ALPHA = 1.0f;
static const float FULL_SCREEN_OVERLAY_MENU_MAX_ALPHA = 0.8f;
static const float FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED = 1.0f/400.0f;

static const float SHINE_EFFECT_X_OFFSET_INIT_VAL = 1.0f;
static const float SHINE_EFFECT_X_OFFSET_END_VAL = -1.0f;

static const float DISSOLVE_EFFECT_Y_INIT_VAL = 1.0f;

static const float NEBULA_ANIMATION_SPEED = 1.0f/15000.0f;

static const float TEXT_FADE_IN_ALPHA_SPEED = 0.002f;

static const glm::vec3 PLAYER_INITIAL_POS = glm::vec3(0.0f, -10.0f, 0.0f);
static const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);
static const glm::vec3 MAP_BACKGROUND_SCALE = glm::vec3(120.0f, 120.0f, 1.0f);

static const glm::vec3 BOSS_HEALTH_BAR_POSITION = glm::vec3(0.0f, 11.5f, 2.5f);
static const glm::vec3 BOSS_HEALTH_BAR_SCALE = glm::vec3(10.0f, 1.0f, 1.0f);

static const glm::vec4 AMBIENT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
static const glm::vec4 POINT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

static const glm::vec2 MAP_MAX_WORLD_BOUNDS = glm::vec2(40.0f, 20.0f);
static const glm::vec2 MAP_MIN_WORLD_BOUNDS = glm::vec2(-17.0f, -13.0f);

enum class LabOptionType
{
    Repair, CrystalTransfer, Research
};

static const std::unordered_map<game_constants::LabOptionType, std::string> LAB_OPTION_TYPE_TO_TEXTURE =
{
    { LabOptionType::Repair, "backgrounds/lab/lab_option_repair.bmp" },
    { LabOptionType::CrystalTransfer, "backgrounds/lab/lab_option_crystal_transfer.bmp" },
    { LabOptionType::Research, "backgrounds/lab/lab_option_research.bmp" }
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* GameConstants_h */
