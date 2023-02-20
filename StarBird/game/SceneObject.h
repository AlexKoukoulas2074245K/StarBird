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

struct SceneObject // 376b
{
    strutils::StringId mObjectFamilyTypeName = strutils::StringId();
    strutils::StringId mNameTag = strutils::StringId();
    strutils::StringId mStateName = scene_object_constants::DEFAULT_SCENE_OBJECT_STATE;
    strutils::StringId mFontName = strutils::StringId();
    std::string mText;
    std::unordered_map<strutils::StringId, bool, strutils::StringIdHasher> mShaderBoolUniformValues;
    std::unordered_map<strutils::StringId, int, strutils::StringIdHasher> mShaderIntUniformValues;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mShaderFloatUniformValues;
    std::unordered_map<strutils::StringId, glm::mat4, strutils::StringIdHasher> mShaderMat4UniformValues;
    std::unique_ptr<IAnimation> mAnimation = nullptr;
    b2Body* mBody = nullptr;
    glm::vec3 mCustomPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mCustomRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mCustomScale = glm::vec3(1.0f, 1.0f, 1.0f);
    resources::ResourceId mShaderEffectTextureResourceId = resources::ResourceId();
    resources::ResourceId mShaderResourceId = resources::ResourceId();
    resources::ResourceId mMeshResourceId = resources::ResourceId();
    SceneObjectType mSceneObjectType = SceneObjectType::WorldGameObject;
    float mHealth = 0;
    bool mInvisible = false;
    bool mUseBodyForRendering = false;
};

///------------------------------------------------------------------------------------------------

struct SceneObjectComparator {
    bool operator()(const SceneObject& lhs, const SceneObject& rhs) const {
        return lhs.mMeshResourceId < rhs.mMeshResourceId;
    }
};

///------------------------------------------------------------------------------------------------

#endif /* SceneObject_h */
