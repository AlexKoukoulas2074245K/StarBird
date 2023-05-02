///------------------------------------------------------------------------------------------------
///  UpgradeDefinition.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradeDefinition_h
#define UpgradeDefinition_h

///------------------------------------------------------------------------------------------------

#include "../../utils/StringUtils.h"
#include "../../resloading/ResourceLoadingService.h"

///------------------------------------------------------------------------------------------------

struct UpgradeDefinition
{
    strutils::StringId mUpgradeNameId = strutils::StringId();
    strutils::StringId mUpgradeDescription = strutils::StringId();
    std::string mTextureFileName = "";
    bool mIntransient = false;
    bool mEventOnly = false;
    bool mEquippable = false;
    bool mUnlocked = false;
    long mDefaultUnlockCost = 0;
    long mCrystalUnlockProgress = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradeDefinition_h */
