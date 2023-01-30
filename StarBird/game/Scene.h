///------------------------------------------------------------------------------------------------
///  Scene.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Scene_h
#define Scene_h

///------------------------------------------------------------------------------------------------

#include "SceneObject.h"
#include "SceneRenderer.h"
#include "../utils/StringUtils.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class Scene final
{
public:
    Scene();
    
    std::optional<std::reference_wrapper<SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectNameTag);
    std::optional<std::reference_wrapper<const SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectNameTag) const;
    
    void AddSceneObject(SceneObject&& sceneObject);
    void RemoveAllSceneObjectsWithNameTag(const strutils::StringId& nameTag);
    
    void UpdateScene();
    void RenderScene();
    
private:
    std::vector<SceneObject> mSceneObjects;
    std::vector<strutils::StringId> mNameTagsOfSceneObjectsToRemove;
    SceneRenderer mSceneRenderer;
};

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
