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
#include "SceneUpdater.h"
#include "SceneRenderer.h"
#include "../utils/StringUtils.h"

#include <optional>
#include <vector>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

class b2World;
class Scene final
{
public:
    Scene();
    
    int GetBodyCount() const;
    
    std::optional<std::reference_wrapper<SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectNameTag);
    std::optional<std::reference_wrapper<const SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectNameTag) const;
    
    void AddSceneObject(SceneObject&& sceneObject);
    void RemoveAllSceneObjectsWithNameTag(const strutils::StringId& nameTag);

    void LoadLevel(const std::string& levelName);
    
    void UpdateScene(const float dtMilis);
    void RenderScene();
    
private:
    void LoadLevelInvariantObjects();
    
private:
    b2World mBox2dWorld;
    std::vector<SceneObject> mSceneObjects;
    std::vector<SceneObject> mSceneObjectsToAdd;
    std::vector<strutils::StringId> mNameTagsOfSceneObjectsToRemove;
    SceneUpdater mSceneUpdater;
    SceneRenderer mSceneRenderer;
    bool mPreFirstUpdate;
};

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
