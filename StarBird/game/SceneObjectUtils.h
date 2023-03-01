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
#include "../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

struct SceneObject;
struct ObjectTypeDefinition;
class b2World;

///------------------------------------------------------------------------------------------------

namespace scene_object_utils
{

///-----------------------------------------------------------------------------------------------
/// Computes and returns whether the given point is inside a scene object.
/// @param[in] sceneObject scene object to test. Will work for both text SOs and Textured SOs
/// @param[in] point point to test.
/// @returns whether the point is inside the scene object's bounds.
bool IsPointInsideSceneObject(const SceneObject& sceneObject, const glm::vec2& point);

///-----------------------------------------------------------------------------------------------
/// Generates and returns a string representation of the scene object body's pointer.
/// @param[in] sceneObject the scene object (containing a body) to generate the name for
/// @returns the string representation of the scene object body's pointer if it owns one, or an empty StringId otherwise.
strutils::StringId GenerateSceneObjectName(const SceneObject& sceneObject);

///-----------------------------------------------------------------------------------------------
/// Creates a scene object having a generic dynamic body.
/// @param[in] objectDef the object definition to draw most fields from
/// @param[in] position the position to initially set for the scee object
/// @param[in] box2dWorld ref to the world, needed for body creation
/// @param[in] sceneObjectName (optional) if this is supplied, the scene object wil be named this, rather than
/// a generated name based on its body pointer.
/// @param[in] bodyCustomScaling (optional) if this is supplied, the body will be sized based on mesh and these dimensions, rather than the texture dimensions alone
/// @returns the constructed scene object
SceneObject CreateSceneObjectWithBody(const ObjectTypeDefinition& objectDef, const glm::vec3& position, b2World& box2dWorld, const strutils::StringId sceneObjectName = strutils::StringId(), glm::vec2 bodyCustomScaling = glm::vec2(0.0f));
                                        
///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObjectUtils_h */
