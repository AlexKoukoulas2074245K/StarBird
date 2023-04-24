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
#include <vector>

///------------------------------------------------------------------------------------------------

class UpgradesLoader: public BaseGameDataLoader
{
public:
    UpgradesLoader();
    std::vector<UpgradeDefinition>& LoadAllUpgrades();
    
private:
    std::vector<UpgradeDefinition> mConstructedUpgrades;
};

///------------------------------------------------------------------------------------------------

#endif /* UpgradesLoader_h */
