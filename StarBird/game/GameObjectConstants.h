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

///------------------------------------------------------------------------------------------------

namespace gameobject_constants
{

///------------------------------------------------------------------------------------------------

static const float WAVE_INTRO_TEXT_SPEED = 0.01f;
static const float WAVE_INTRO_DURATION_MILIS = 5000.0f;
static const float JOYSTICK_Z = 1.0f;
static const float JOYSTICK_BOUNDS_Z = 2.0f;
static const float BACKGROUND_Z = -1.0f;

static const glm::vec2 PLAYER_INITIAL_POS = glm::vec2(0.0f, -10.0f);
static const glm::vec3 BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);
static const glm::vec3 JOYSTICK_SCALE = glm::vec3(2.0f, 2.0f, 1.0f);
static const glm::vec3 JOYSTICK_BOUNDS_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);
static const glm::vec3 WAVE_INTRO_TEXT_INIT_POS = glm::vec3(-20.0f, 0.0f, 5.0f);
static const glm::vec3 WAVE_INTRO_TEXT_SCALE = glm::vec3(0.03f, 0.03f, 1.0f);

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* GameObjectConstants_h */
