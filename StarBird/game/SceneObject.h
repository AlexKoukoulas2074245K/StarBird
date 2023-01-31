///------------------------------------------------------------------------------------------------
///  SceneObject.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneObject_h
#define SceneObject_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "../resloading/ResourceLoadingService.h"
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class b2Body;

///------------------------------------------------------------------------------------------------

enum class SceneObjectType
{
    GameObject, GUIObject
};

///------------------------------------------------------------------------------------------------

struct SceneObject // 208b
{
    strutils::StringId mObjectFamilyTypeName = strutils::StringId();
    strutils::StringId mNameTag = strutils::StringId();
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mShaderFloatUniformValues;
    std::unordered_map<strutils::StringId, glm::mat4, strutils::StringIdHasher> mShaderMat4UniformValues;
    b2Body* mBody = nullptr;
    glm::vec3 mCustomPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mCustomRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mCustomScale = glm::vec3(1.0f, 1.0f, 1.0f);
    resources::ResourceId mTextureResourceId = resources::ResourceId();
    resources::ResourceId mShaderResourceId = resources::ResourceId();
    resources::ResourceId mMeshResourceId = resources::ResourceId();
    SceneObjectType mSceneObjectType = SceneObjectType::GameObject;
    long mHealth = 0;
    bool mInvisible = false;
};

///------------------------------------------------------------------------------------------------

struct SceneObjectComparator {
    bool operator()(const SceneObject& lhs, const SceneObject& rhs) const {
        return lhs.mTextureResourceId < rhs.mTextureResourceId;
    }
};

///------------------------------------------------------------------------------------------------

#endif /* SceneObject_h */
