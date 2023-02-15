///------------------------------------------------------------------------------------------------
///  UpgradesLogicHandler.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradesLogicHandler_h
#define UpgradesLogicHandler_h

///------------------------------------------------------------------------------------------------

#include "../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

class Scene;
class LevelUpdater;

///------------------------------------------------------------------------------------------------

class UpgradesLogicHandler final
{
public:
    UpgradesLogicHandler(Scene& scene, LevelUpdater& levelUpdater);
    
    // Relevant game event callbacks
    void OnUpgradeEquipped(const strutils::StringId& upgradeId);
    void OnUpdate(const float dtMillis);

private:
    void CreateMirrorImageSceneObjects();
    void CreatePlayerShieldSceneObject();
    
    void UpdateMirrorImages(const float dtMillis);
    void UpdatePlayerShield(const float dtMillis);
    
private:
    Scene& mScene;
    LevelUpdater& mLevelUpdater;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradesLogicHandler_h */
