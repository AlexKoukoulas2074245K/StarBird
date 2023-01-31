///------------------------------------------------------------------------------------------------
///  SceneRenderer.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneRenderer_h
#define SceneRenderer_h

///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "SceneObject.h"
#include <vector>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class SceneRenderer final
{
public:
    SceneRenderer();
    
    void Render(const std::vector<SceneObject>& sceneObjects, const std::unordered_map<SceneObjectType, Camera>& sceneObjectTypeToCamera);
    
};

///------------------------------------------------------------------------------------------------

#endif /* SceneRenderer_h */
