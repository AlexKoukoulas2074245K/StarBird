///------------------------------------------------------------------------------------------------
///  ObjectTypeDefinition.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ObjectTypeDefinition_h
#define ObjectTypeDefinition_h

///------------------------------------------------------------------------------------------------

#include "../Animation.h"
#include "../../utils/MathUtils.h"
#include "../../utils/StringUtils.h"
#include "../../resloading/ResourceLoadingService.h"

#include <Box2D/Dynamics/b2Fixture.h>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

enum class MovementControllerPattern
{
    NONE, CUSTOM_VELOCITY, CHASING_PLAYER, INPUT_CONTROLLED
};

///------------------------------------------------------------------------------------------------

struct ObjectTypeDefinition
{
    strutils::StringId mName = strutils::StringId();
    float mHealth = 0;
    float mDamage = 0;
    resources::ResourceId mShaderResourceId = resources::ResourceId();
    resources::ResourceId mMeshResourceId = resources::ResourceId();
    mutable std::unordered_map<strutils::StringId, Animation, strutils::StringIdHasher> mAnimations;
    b2Filter mContactFilter;
    glm::vec2 mConstantLinearVelocity = glm::vec2(0.0f, 0.0f);
    float mSpeed = 0.0f;
    float mDensity = 0.0f;
    float mSize = 0.0f;
    float mLinearDamping = 0.0f;
    MovementControllerPattern mMovementControllerPattern = MovementControllerPattern::NONE;
};

///------------------------------------------------------------------------------------------------

#endif /* ObjectTypeDefinition_h */
