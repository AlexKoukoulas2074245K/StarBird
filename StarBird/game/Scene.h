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
#include "IUpdater.h"
#include "SceneRenderer.h"

#include "datarepos/LightRepository.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <optional>
#include <vector>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

class IUpdater;
class Scene final
{
public:
    Scene();
    ~Scene();
    
    std::string GetSceneStateDescription() const;
    
    std::optional<std::reference_wrapper<SceneObject>> GetSceneObject(const b2Body* body);
    std::optional<std::reference_wrapper<const SceneObject>> GetSceneObject(const b2Body* body) const;
    std::optional<std::reference_wrapper<SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectName);
    std::optional<std::reference_wrapper<const SceneObject>> GetSceneObject(const strutils::StringId& sceneObjectName) const;
    const std::vector<SceneObject>& GetSceneObjects() const;
    
    const LightRepository& GetLightRepository() const;
    LightRepository& GetLightRepository();
    
    void AddSceneObject(SceneObject&& sceneObject);
    void RemoveAllSceneObjectsWithName(const strutils::StringId& name);

    void LoadLevel(const std::string& levelName);
    
    void OnAppStateChange(Uint32 event);
    void UpdateScene(const float dtMillis);
    void RenderScene();
    
    void SetSceneRendererPhysicsDebugMode(const bool debugMode);
    
#ifdef DEBUG
    void OpenDebugConsole();
#endif
    
private:
    b2World mBox2dWorld;
    std::vector<SceneObject> mSceneObjects;
    std::vector<SceneObject> mSceneObjectsToAdd;
    std::vector<strutils::StringId> mNamesOfSceneObjectsToRemove;
    LightRepository mLightRepository;
    std::unique_ptr<IUpdater> mSceneUpdater;
    SceneRenderer mSceneRenderer;
    bool mPreFirstUpdate;
};

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
