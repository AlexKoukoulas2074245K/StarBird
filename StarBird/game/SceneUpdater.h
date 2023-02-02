///------------------------------------------------------------------------------------------------
///  SceneUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneUpdater_h
#define SceneUpdater_h

///------------------------------------------------------------------------------------------------

#include "SceneObject.h"
#include "LevelDefinition.h"
#include "RepeatableFlow.h"
#include "../utils/StringUtils.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinition;
class Scene;
class b2World;
class SceneUpdater final
{
public:
    SceneUpdater(Scene& scene, b2World& box2dWorld);
    
    void SetLevelProperties(LevelDefinition&& levelDef);
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMilis);
    
private:
    void UpdateAnimation(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis);
    void UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis);
    
private:
    Scene& mScene;
    b2World& mBox2dWorld;
    LevelDefinition mLevel;
    std::vector<RepeatableFlow> mFlows;
    bool mAllowInputControl;
};

///------------------------------------------------------------------------------------------------

#endif /* SceneUpdater_h */
