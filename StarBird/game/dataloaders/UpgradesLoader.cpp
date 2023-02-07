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
            upgrade.mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + texture->value() + ".bmp");
        }
        
        auto* description = node->first_attribute("description");
        if (description)
        {
            upgrade.mUpgradeDescription = strutils::StringId(description->value());
        }
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            upgrade.mUpgradeName = strutils::StringId(name->value());
            mConstructedUpgrades[upgrade.mUpgradeName] = std::move(upgrade);
        }
    });
}

///------------------------------------------------------------------------------------------------

std::map<strutils::StringId, UpgradeDefinition> UpgradesLoader::LoadAllUpgrades()
{
    BaseGameDataLoader::LoadData("upgrades");
    return mConstructedUpgrades;
}

///------------------------------------------------------------------------------------------------
