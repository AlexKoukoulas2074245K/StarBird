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
#include "SceneObjectConstants.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "../resloading/ResourceLoadingService.h"

#include <memory>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class b2Body;
class IAnimation;

///------------------------------------------------------------------------------------------------

enum class SceneObjectType
{
    WorldGameObject, GUIObject
};

///------------------------------------------------------------------------------------------------

struct SceneObject // 456b
{
    strutils::StringId mObjectFamilyTypeName = strutils::StringId();
    strutils::StringId mName = strutils::StringId();
    strutils::StringId mStateName = scene_object_constants::DEFAULT_SCENE_OBJECT_STATE;
    strutils::StringId mFontName = strutils::StringId();
    std::string mText;
    std::unordered_map<strutils::StringId, bool, strutils::StringIdHasher> mShaderBoolUniformValues;
    std::unordered_map<strutils::StringId, int, strutils::StringIdHasher> mShaderIntUniformValues;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mShaderFloatUniformValues;
    std::unordered_map<strutils::StringId, glm::vec4, strutils::StringIdHasher> mShaderFloatVec4UniformValues;
    std::unordered_map<strutils::StringId, glm::mat4, strutils::StringIdHasher> mShaderMat4UniformValues;
    std::unique_ptr<IAnimation> mAnimation = nullptr;
    b2Body* mBody = nullptr;
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mScale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 mBodyCustomScale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 mBodyCustomOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    resources::ResourceId mShaderEffectTextureResourceId = resources::ResourceId();
    SceneObjectType mSceneObjectType = SceneObjectType::WorldGameObject;
    float mHealth = 0;
    bool mInvisible = false;
    bool mUseBodyForRendering = false;
};

///------------------------------------------------------------------------------------------------

#endif /* SceneObject_h */
