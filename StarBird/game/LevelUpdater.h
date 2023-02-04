///------------------------------------------------------------------------------------------------
///  LevelUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LevelUpdater_h
#define LevelUpdater_h

///------------------------------------------------------------------------------------------------

#include "SceneObject.h"
#include "LevelDefinition.h"
#include "RepeatableFlow.h"
#include "../utils/StringUtils.h"

#include <unordered_set>
#include <vector>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinition;
class Scene;
class b2World;
class LevelUpdater final
{
public:
    LevelUpdater(Scene& scene, b2World& box2dWorld);
    
    void InitLevel(LevelDefinition&& levelDef);
    void Update(std::vector<SceneObject>& sceneObjects, const float dtMilis);
    
    size_t GetWaveEnemyCount() const;
    
private:
    void UpdateAnimation(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis);
    void UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis);
    
    void CreateWaveIntroText();
    void CreateWave();
    
private:
    enum class LevelState
    {
        WAVE_INTRO,
        FIGHTING_WAVE,
        FINISHED_LEVEL
    };
    
    Scene& mScene;
    b2World& mBox2dWorld;
    LevelDefinition mLevel;
    std::vector<RepeatableFlow> mFlows;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mWaveEnemies;
    size_t mCurrentWaveNumber;
    LevelState mState;
    bool mAllowInputControl;
};

///------------------------------------------------------------------------------------------------

#endif /* LevelUpdater_h */
