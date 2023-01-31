///------------------------------------------------------------------------------------------------
///  SceneUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneUpdater_h
#define SceneUpdater_h

///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "InputContext.h"
#include "SceneObject.h"
#include "GameObjectDefinition.h"
#include "LevelDefinition.h"
#include "../utils/StringUtils.h"

#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;
class SceneUpdater final
{
public:
    SceneUpdater(Scene& scene);
    
    void SetLevelProperties(LevelDefinition&& levelDef, std::unordered_map<strutils::StringId, GameObjectDefinition, strutils::StringIdHasher>&& enemyTypesToDefinitions);
    void Update(std::vector<SceneObject>& sceneObjects, const std::unordered_map<SceneObjectType, Camera>& sceneObjectTypeToCamera, const float dtMilis, const InputContext& inputContext);
    
private:
    void UpdateInputControlledSceneObject(SceneObject& sceneObject, GameObjectDefinition& sceneObjectFamilyDef, const std::unordered_map<SceneObjectType, Camera>& sceneObjectTypeToCamera, const float dtMilis, const InputContext& inputContext);
    
private:
    Scene& mScene;
    LevelDefinition mLevel;
    std::unordered_map<strutils::StringId, GameObjectDefinition, strutils::StringIdHasher> mEnemyTypesToDefinitions;
};

///------------------------------------------------------------------------------------------------

#endif /* SceneUpdater_h */
