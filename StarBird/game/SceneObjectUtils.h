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
/// Changes the state of a scene object and assigns the respective animation for that state.
/// @param[in] sceneObject scene object to change its state.
/// @param[in] objectDef the object definition to get the animation data from.
/// @param[in] newStateName name of the new state to transition the object to.
void ChangeSceneObjectState(SceneObject& sceneObject, const ObjectTypeDefinition& objectDef, const strutils::StringId newStateName);

///-----------------------------------------------------------------------------------------------
/// Computes and returns the bounding rect for a scene object.
/// @param[in] sceneObject scene object to compute the bounding rect for.
/// @param[out] rectBotLeft the bottom-left xy coordinates of the bounding rect.
/// @param[out] rectTopRight the top-right xy coordinates of the bounding rect.
void GetSceneObjectBoundingRect(const SceneObject& sceneObject, glm::vec2& rectBotLeft, glm::vec2& rectTopRight);

///-----------------------------------------------------------------------------------------------
/// Tests the sceen object's name to assess whether it is part of a boss
/// @param[in] sceneObject the scene object to check whether it is a boss part or not
/// @returns whether or not the scene object is part of a boss
bool IsSceneObjectBossPart(const SceneObject& sceneObject);

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
SceneObject CreateSceneObjectWithBody(const ObjectTypeDefinition& objectDef, const glm::vec3& position, b2World& box2dWorld, const strutils::StringId sceneObjectName = strutils::StringId());
                                        
///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObjectUtils_h */
