///------------------------------------------------------------------------------------------------
///  GameObjectConstants.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameObjectConstants_h
#define GameObjectConstants_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

namespace game_object_constants
{

///------------------------------------------------------------------------------------------------

static const strutils::StringId PLAYER_BULLET_FLOW_NAME = strutils::StringId("PLAYER_BULLET_FLOW");
static const strutils::StringId PLAYER_DAMAGE_INVINCIBILITY_FLOW_NAME = strutils::StringId("PLAYER_INVINCIBILITY_FLOW");
static const strutils::StringId WAVE_INTRO_FLOW_NAME = strutils::StringId("WAVE_INTRO_FLOW");

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

static const float FULL_SCREEN_OVERLAY_DARKENING_SPEED = 1.0f/400.0f;

static const float UPGRADE_MOVEMENT_SPEED = 1.0f/400.0f;

static const float WAVE_INTRO_DURATION_MILLIS = 3000.0f;

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

static const float PLAYER_HEALTH_BAR_Z = 0.5f;
static const float PLAYER_HEALTH_LOST_BAR_Z = 0.4f;
static const float HEALTH_LOST_SPEED = 0.001f;

static const float FULL_SCREEN_OVERLAY_MAX_ALPHA = 0.8f;

static const float PLAYER_BULLET_X_OFFSET = 0.48f;

static const float MIRROR_IMAGE_BULLET_X_OFFSET = 0.38f;

static const float HEALTH_POTION_HEALTH_GAIN = 5.0f;

static const float PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR = 1.0/50.0f;
static const float PLAYER_PULSE_SHIELD_ANIM_SPEED = 0.01f;

static const float SELECTED_UPGRADE_PULSE_ENLARGEMENT_FACTOR = 1.0/5.0f;
static const float SELECTED_UPGRADE_PULSE_ANIM_SPEED = 0.03f;

static const float PLAYER_MOVEMENT_ROLL_CHANCE = 0.333f;
static const float PLAYER_MOVEMENT_ROLL_SPEED = 0.008f;
static const float PLAYER_MOVEMENT_ROLL_ANGLE = 180.0f;

static const float EXPLOSION_LIGHT_POWER = 1.0f;
static const float EXPLOSION_LIGHT_FADE_SPEED = 1.0f/400.0f;

static const glm::vec3 PLAYER_INITIAL_POS = glm::vec3(0.0f, -10.0f, 0.0f);
static const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);

static const glm::vec3 JOYSTICK_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);
static const glm::vec3 JOYSTICK_BOUNDS_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);

static const glm::vec3 WAVE_INTRO_TEXT_INIT_POS = glm::vec3(-3.0f, 0.0f, 2.0f);
static const glm::vec3 WAVE_INTRO_TEXT_SCALE = glm::vec3(0.02f, 0.02f, 1.0f);

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
static const glm::vec3 FULL_SCREEN_OVERLAY_SCALE = glm::vec3(80.0f, 80.0f, 1.0f);

static const glm::vec3 HEALTH_BAR_POSITION = glm::vec3(0.0f, -12.0f, 0.5f);
static const glm::vec3 HEALTH_BAR_SCALE = glm::vec3(8.0f, 1.0f, 1.0f);

static const glm::vec3 LEFT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(-2.0f, -0.5f, 0.0f);
static const glm::vec3 LEFT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 RIGHT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(2.0f, -0.5f, 0.0f);
static const glm::vec3 RIGHT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 PLAYER_SHIELD_POSITION_OFFSET = glm::vec3(0.0f, 0.5f, 0.5f);
static const glm::vec3 PLAYER_SHIELD_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);

static const glm::vec4 AMBIENT_LIGHT_COLOR = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
static const glm::vec4 POINT_LIGHT_COLOR = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* GameObjectConstants_h */
