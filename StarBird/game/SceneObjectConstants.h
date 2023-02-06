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

namespace sceneobject_constants
{

///------------------------------------------------------------------------------------------------


static const strutils::StringId DEFAULT_SCENE_OBJECT_STATE = strutils::StringId("idle");
static const strutils::StringId PLAYER_SCENE_OBJECT_NAME = strutils::StringId("PLAYER");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("BG");
static const strutils::StringId JOYSTICK_SCENE_OBJECT_NAME = strutils::StringId("JOYSTICK");
static const strutils::StringId JOYSTICK_BOUNDS_SCENE_OBJECT_NAME = strutils::StringId("JOYSTICK_BOUNDS");
static const strutils::StringId WAVE_INTRO_TEXT_SCNE_OBJECT_NAME = strutils::StringId("WAVE_INTRO_TEXT");
static const strutils::StringId LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("LEFT_UPGRADE_CONTAINER");
static const strutils::StringId RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("RIGHT_UPGRADE_CONTAINER");
static const strutils::StringId UPGRADE_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("UPGRADE_OVERLAY");


static const strutils::StringId TEXTURE_OFFSET_UNIFORM_NAME = strutils::StringId("tex_offset");
static const strutils::StringId IS_TEXTURE_SHEET_UNIFORM_NAME = strutils::StringId("texture_sheet");
static const strutils::StringId MIN_U_UNIFORM_NAME = strutils::StringId("min_u");
static const strutils::StringId MIN_V_UNIFORM_NAME = strutils::StringId("min_v");
static const strutils::StringId MAX_U_UNIFORM_NAME = strutils::StringId("max_u");
static const strutils::StringId MAX_V_UNIFORM_NAME = strutils::StringId("max_v");
static const strutils::StringId CUSTOM_ALPHA_UNIFORM_NAME = strutils::StringId("custom_alpha");

static const char* QUAD_MESH_FILE_NAME = "quad.obj";
static const char* BASIC_SHADER_FILE_NAME = "basic.vs";
static const char* TEXTURE_OFFSET_SHADER_FILE_NAME = "tex_offset.vs";
static const char* CUSTOM_ALPHA_SHADER_FILE_NAME = "custom_alpha.vs";
static const char* BACKGROUND_TEXTURE_FILE_NAME = "space_bg.bmp";
static const char* PLAYER_TEXTURE_FILE_NAME = "player.bmp";
static const char* BULLET_TEXTURE_FILE_NAME = "bullet.bmp";
static const char* JOYSTICK_TEXTURE_FILE_NAME = "joystick.bmp";
static const char* JOYSTICK_BOUNDS_TEXTURE_FILE_NAME = "joystick_bounds.bmp";
static const char* UPGRADE_CONTAINER_TEXTURE_FILE_NAME = "upgrade_container.bmp";
static const char* UPGRADE_OVERLAY_TEXTURE_FILE_NAME = "overlay.bmp";

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObjectConstants_h */
