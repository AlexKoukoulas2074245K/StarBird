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
    WorldGameObject, GUIGameObject
};

///------------------------------------------------------------------------------------------------

struct SceneObject // 72b
{
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mShaderFloatUniformValues;
    std::unordered_map<strutils::StringId, glm::mat4, strutils::StringIdHasher> mShaderMat4UniformValues;
    strutils::StringId mNameTag;
    b2Body* mBody;
    glm::vec3 mCustomPosition;
    glm::vec3 mCustomRotation;
    glm::vec3 mCustomScale;
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mShaderResourceId;
    resources::ResourceId mMeshResourceId;
    SceneObjectType mSceneObjectType;
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
