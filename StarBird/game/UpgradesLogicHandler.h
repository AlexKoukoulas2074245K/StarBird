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
#include "RepeatableFlow.h"

#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;

///------------------------------------------------------------------------------------------------

class UpgradesLogicHandler final
{
public:
    UpgradesLogicHandler(Scene& scene);
    
    void InitializeEquippedUpgrade(const strutils::StringId& upgradeId);
    void AnimateUpgradeGained(const strutils::StringId& upgradeId);
    
    void Update(const float dtMillis);

private:
    void CreateMirrorImageSceneObjects();
    void CreatePlayerShieldSceneObject();
    
    void AnimateCrystalGiftUpgradeGained();
    
    void UpdateMirrorImages(const float dtMillis);
    void UpdatePlayerShield(const float dtMillis);
    
private:
    Scene& mScene;
    std::vector<RepeatableFlow> mFlows;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradesLogicHandler_h */
