///------------------------------------------------------------------------------------------------
///  Scene.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Scene_h
#define Scene_h

///------------------------------------------------------------------------------------------------

#include "FullScreenOverlayController.h"
#include "IUpdater.h"
#include "SceneObject.h"
#include "SceneRenderer.h"


#include "datarepos/LightRepository.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <optional>
#include <vector>
#include <unordered_set>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

class IUpdater;
class Scene final
{
public:
    enum class SceneType
    {
        MAP, LAB, STATS_UPGRADE, LEVEL, CHEST_REWARD
    };
    
    class TransitionParameters
    {
    public:
        TransitionParameters(const SceneType sceneType, const std::string& sceneNameToTransitionTo, const bool useOverlay)
            : mSceneType(sceneType)
            , mSceneNameToTransitionTo(sceneNameToTransitionTo)
            , mUseOverlay(useOverlay)
        {
            
        }
        
        const SceneType mSceneType;
        const std::string mSceneNameToTransitionTo;
        const bool mUseOverlay;
    };
    
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
    
    void AddOverlayController(const float darkeningSpeed, const float maxDarkeningValue, const bool pauseAtMidPoint, FullScreenOverlayController::CallbackType midwayCallback = nullptr, FullScreenOverlayController::CallbackType completionCallback = nullptr);
    void ResumeOverlayController();
    
    void AddSceneObject(SceneObject&& sceneObject);
    void RemoveAllSceneObjectsWithName(const strutils::StringId& name);

    void ChangeScene(const TransitionParameters& transitionParameters);
    
    void OnAppStateChange(Uint32 event);
    void UpdateScene(const float dtMillis);
    void UpdateCrossSceneInterfaceObjects(const float dtMillis);
    void UpdateOnSceneEditModeOn(const float dtMillis);
    
    void RenderScene();
    
    void SetSceneRendererPhysicsDebugMode(const bool debugMode);
    void SetSceneEditMode(const bool editMode);
    void SetSceneEditResultMessage(const glm::vec3& position, const glm::vec3& scale);
    
#ifdef DEBUG
    void OpenDebugConsole();
#endif
    
    void CreateCrossSceneInterfaceObjects();
    
private:
    b2World mBox2dWorld;
    std::unordered_set<resources::ResourceId> mAccumulatedResourcesForScene;
    std::vector<SceneObject> mSceneObjects;
    std::vector<SceneObject> mSceneObjectsToAdd;
    std::vector<strutils::StringId> mNamesOfSceneObjectsToRemove;
    LightRepository mLightRepository;
    std::unique_ptr<IUpdater> mSceneUpdater;
    std::unique_ptr<FullScreenOverlayController> mOverlayController;
    std::unique_ptr<TransitionParameters> mTransitionParameters;
    SceneRenderer mSceneRenderer;
    bool mPreFirstUpdate;
    bool mSceneEditMode;
};

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
