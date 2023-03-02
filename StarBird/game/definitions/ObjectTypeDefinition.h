///------------------------------------------------------------------------------------------------
///  ObjectTypeDefinition.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ObjectTypeDefinition_h
#define ObjectTypeDefinition_h

///------------------------------------------------------------------------------------------------

#include "../Animations.h"
#include "../../utils/MathUtils.h"
#include "../../utils/StringUtils.h"
#include "../../resloading/ResourceLoadingService.h"

#include <Box2D/Dynamics/b2Fixture.h>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

enum class MovementControllerPattern
{
    NONE, CONSTANT_VELOCITY, CHASING_PLAYER, INPUT_CONTROLLED
};

///------------------------------------------------------------------------------------------------

enum class FlippedDisplay
{
    NONE, FLIPPED_X, FLIPPED_Y, FLIPPED_XY
};

///------------------------------------------------------------------------------------------------

struct ObjectTypeDefinition
{    
    strutils::StringId mName = strutils::StringId();
    strutils::StringId mProjectileType = strutils::StringId();
    float mHealth = 0;
    float mDamage = 0;
    std::unordered_map<strutils::StringId, IAnimation*, strutils::StringIdHasher> mAnimations;
    b2Filter mContactFilter;
    glm::vec2 mConstantLinearVelocity = glm::vec2(0.0f, 0.0f);
    float mSpeed = 0.0f;
    float mSize = 0.0f;
    float mLinearDamping = 0.0f;
    float mShootingFrequencyMillis = 0.0f;
    MovementControllerPattern mMovementControllerPattern = MovementControllerPattern::NONE;
    FlippedDisplay mFlippedDisplay = FlippedDisplay::NONE;
};

///------------------------------------------------------------------------------------------------

#endif /* ObjectTypeDefinition_h */
