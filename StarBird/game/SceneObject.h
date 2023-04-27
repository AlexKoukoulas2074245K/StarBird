///------------------------------------------------------------------------------------------------
///  SceneObject.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneObject_h
#define SceneObject_h

///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "../resloading/ResourceLoadingService.h"

#include <memory>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

class b2Body;
class BaseAnimation;

///------------------------------------------------------------------------------------------------

enum class SceneObjectType
{
    WorldGameObject, GUIObject
};

///------------------------------------------------------------------------------------------------

struct SceneObject // 472b
{
    // ObjectTypeDefinition name with family attributes
    strutils::StringId mObjectFamilyTypeName = strutils::StringId();
    
    // Scene Object name to be polled with Via the Scene's methods
    strutils::StringId mName = strutils::StringId();
    
    // Current state of the scene object. Can be used to select mapped animations in parent ObjectTypeDefinition
    strutils::StringId mStateName = game_constants::DEFAULT_SCENE_OBJECT_STATE;
    
    // Font name that this text scene object (and only text scene objects have this) should use
    strutils::StringId mFontName = strutils::StringId();
    
    // Text that this scene object will render
    std::string mText;
    
    // Shader uniforms that can be set directly on these containers
    std::unordered_map<strutils::StringId, bool, strutils::StringIdHasher> mShaderBoolUniformValues;
    std::unordered_map<strutils::StringId, int, strutils::StringIdHasher> mShaderIntUniformValues;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mShaderFloatUniformValues;
    std::unordered_map<strutils::StringId, glm::vec4, strutils::StringIdHasher> mShaderFloatVec4UniformValues;
    std::unordered_map<strutils::StringId, glm::mat4, strutils::StringIdHasher> mShaderMat4UniformValues;
    
    // Current animation of the scene object. Current texture, mesh and shader will be checked by the Renderer
    // from this animation
    std::unique_ptr<BaseAnimation> mAnimation = nullptr;
    
    // Extra animations that are used in conjunction with the above to provide more than one transformations
    // to the scene object. E.g. Pulsing + Rotation animations
    std::vector<std::unique_ptr<BaseAnimation>> mExtraCompoundingAnimations;
    
    // Box2D physical body
    b2Body* mBody = nullptr;
    
    // Scene object tranform (position is always inferred by the body above in terms of rendering position - rotation and scale will not).
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mScale = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Scale and offset overrides when we want the physical body to be located and scaled differently
    glm::vec3 mBodyCustomScale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 mBodyCustomOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Scene object type World/GUI
    SceneObjectType mSceneObjectType = SceneObjectType::WorldGameObject;
    
    // Timer used for activating player chasing mode
    float mDormantMillis = 0.0f;
    
    // A health value to be used by game entities
    float mHealth = 0;
    
    // Whether or not rendering will be skipped for this scene object.
    bool mInvisible = false;
    
    // Whether or not this scene object should be excluded form health calculations
    bool mInvulnerable = false;
    
    // Whether or not the parent (ObjectTypeDefinition's) movement patterns should not be followed
    bool mCustomDrivenMovement = false;
    
    // Whether or not this scene object should stay alive cross scene creation/destruction
    bool mCrossSceneLifetime = false;
    
    // Whether or not this scene object has been selected in edit mode
    bool mDebugEditSelected = false;
};

///------------------------------------------------------------------------------------------------

#endif /* SceneObject_h */
