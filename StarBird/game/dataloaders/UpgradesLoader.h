///------------------------------------------------------------------------------------------------
///  UpgradesLoader.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 06/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef UpgradesLoader_h
#define UpgradesLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "../definitions/UpgradeDefinition.h"

#include <string>
#include <map>

///------------------------------------------------------------------------------------------------

class UpgradesLoader: public BaseGameDataLoader
{
public:
    UpgradesLoader();
    std::map<strutils::StringId, UpgradeDefinition> LoadAllUpgrades();
    
private:
    std::map<strutils::StringId, UpgradeDefinition> mConstructedUpgrades;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradesLoader_h */
