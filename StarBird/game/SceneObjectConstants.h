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

static const strutils::StringId PLAYER_SCENE_OBJECT_NAME("PLAYER");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME("BG");
static const strutils::StringId JOYSTICK_SCENE_OBJECT_NAME("JOYSTICK");
static const strutils::StringId JOYSTICK_BOUNDS_SCENE_OBJECT_NAME("JOYSTICK_BOUNDS");

static const char* QUAD_MESH_FILE_NAME = "quad.obj";
static const char* BASIC_SHADER_FILE_NAME = "basic.vs";
static const char* TEXTURE_OFFSET_SHADER_FILE_NAME = "texoffset.vs";
static const char* BACKGROUND_TEXTURE_FILE_NAME = "space_bg.bmp";
static const char* PLAYER_TEXTURE_FILE_NAME = "player.bmp";
static const char* BULLET_TEXTURE_FILE_NAME = "bullet.bmp";
static const char* WALLS_TEXTURE_FILE_NAME = "debug.bmp";
static const char* JOYSTICK_TEXTURE_FILE_NAME = "joystick.bmp";
static const char* JOYSTICK_BOUNDS_TEXTURE_FILE_NAME = "joystick_bounds.bmp";


///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObjectConstants_h */
