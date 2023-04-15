///------------------------------------------------------------------------------------------------
///  UpgradesLevelLogicHandler.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradesLevelLogicHandler_h
#define UpgradesLevelLogicHandler_h

///------------------------------------------------------------------------------------------------

#include "../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

class Scene;

///------------------------------------------------------------------------------------------------

class UpgradesLevelLogicHandler final
{
public:
    UpgradesLevelLogicHandler(Scene& scene);
    
    void InitializeEquippedUpgrade(const strutils::StringId& upgradeId);
    
    void Update(const float dtMillis);

private:
    void CreateMirrorImageSceneObjects();
    void CreatePlayerShieldSceneObject();
    
    void UpdateMirrorImages(const float dtMillis);
    void UpdatePlayerShield(const float dtMillis);
    
private:
    Scene& mScene;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradesLevelLogicHandler_h */
