///------------------------------------------------------------------------------------------------
///  UpgradesLoader.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 06/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradesLoader.h"
#include "../../resloading/ResourceLoadingService.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

UpgradesLoader::UpgradesLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Upgrade"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        UpgradeDefinition upgrade;
        
        auto* texture = node->first_attribute("texture");
        if (texture)
        {
            upgrade.mTextureFileName =  texture->value() + std::string(".bmp");
        }
        
        auto* description = node->first_attribute("description");
        if (description)
        {
            upgrade.mUpgradeDescription = strutils::StringId(description->value());
        }
        
        auto* intransient = node->first_attribute("intransient");
        if (intransient)
        {
            upgrade.mIntransient =  strcmp(intransient->value(), "true") == 0;
        }
        
        auto* eventOnly = node->first_attribute("eventOnly");
        if (eventOnly)
        {
            upgrade.mEventOnly =  strcmp(eventOnly->value(), "true") == 0;
        }
        
        auto* equippable = node->first_attribute("equippable");
        if (equippable)
        {
            upgrade.mEquippable = strcmp(equippable->value(), "true") == 0;
        }
        
        auto* unlockedByDefault = node->first_attribute("unlockedByDefault");
        if (unlockedByDefault)
        {
            upgrade.mUnlocked = strcmp(unlockedByDefault->value(), "true") == 0;
        }
        
        auto* unlockCost = node->first_attribute("unlockCost");
        if (unlockCost)
        {
            upgrade.mDefaultUnlockCost = std::stoi(unlockCost->value());
            upgrade.mCrystalUnlockProgress = 0;
        }
        
        auto* upgradeNameId = node->first_attribute("nameId");
        if (upgradeNameId)
        {
            upgrade.mUpgradeNameId = strutils::StringId(upgradeNameId->value());
            mConstructedUpgrades.push_back(upgrade);
        }
    });
}

///------------------------------------------------------------------------------------------------

std::vector<UpgradeDefinition>& UpgradesLoader::LoadAllUpgrades()
{
    BaseGameDataLoader::LoadData("upgrades");
    return mConstructedUpgrades;
}

///------------------------------------------------------------------------------------------------
