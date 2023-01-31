///------------------------------------------------------------------------------------------------
///  PhysicsConstants.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef PhysicsConstants_h
#define PhysicsConstants_h

///------------------------------------------------------------------------------------------------

#include <Box2D/Common/b2Settings.h>

///------------------------------------------------------------------------------------------------

static constexpr uint16 ENEMY_CATEGORY_BIT  = 0x1;
static constexpr uint16 ENEMY_BULLET_CATEGORY_BIT = 0x2;
static constexpr uint16 PLAYER_CATEGORY_BIT = 0x4;
static constexpr uint16 PLAYER_BULLET_CATEGORY_BIT = 0x8;
static constexpr uint16 LEVEL_BOUNDS_CATEGORY_BIT = 0x10;

///------------------------------------------------------------------------------------------------

#endif /* PhysicsConstants_h */
