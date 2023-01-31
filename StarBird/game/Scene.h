///------------------------------------------------------------------------------------------------
///  Scene.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Scene_h
#define Scene_h

///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "InputContext.h"
#include "SceneObject.h"
#include "SceneUpdater.h"
#include "SceneRenderer.h"
#include "../utils/StringUtils.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class b2World;
class Scene final
{
public:
    Scene(b2World& world);
    
    std::optional<std::reference_wrapper<SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectNameTag);
    std::optional<std::reference_wrapper<const SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectNameTag) const;
    
    void AddSceneObject(SceneObject&& sceneObject);
    void RemoveAllSceneObjectsWithNameTag(const strutils::StringId& nameTag);

    void LoadLevel(const std::string& levelName);
    
    void UpdateScene(const float dtMilis, const InputContext& inputContext);
    void RenderScene();
    
private:
    b2World& mWorld;
    std::vector<SceneObject> mSceneObjects;
    std::vector<SceneObject> mSceneObjectsToAdd;
    std::vector<strutils::StringId> mNameTagsOfSceneObjectsToRemove;
    std::unordered_map<SceneObjectType, Camera> mSceneObjectTypeToCamera;
    SceneUpdater mSceneUpdater;
    SceneRenderer mSceneRenderer;
    bool mPreFirstUpdate;
};

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
