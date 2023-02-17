///------------------------------------------------------------------------------------------------
///  SceneObjectUtils.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneObjectUtils_h
#define SceneObjectUtils_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"

///------------------------------------------------------------------------------------------------

struct SceneObject;

///------------------------------------------------------------------------------------------------

namespace scene_object_utils
{

///-----------------------------------------------------------------------------------------------
/// Computes and returns whether the given point is inside a scene object.
/// @param[in] sceneObject scene object to test. Will work for both text SOs and Textured SOs
/// @param[in] point point to test.
/// @returns whether the point is inside the scene object's bounds.
bool IsPointInsideSceneObject(const SceneObject& sceneObject, const glm::vec2& point);

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObjectUtils_h */
