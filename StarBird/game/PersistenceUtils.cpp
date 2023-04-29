///------------------------------------------------------------------------------------------------
///  PersistenceUtils.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "../utils/ObjectiveCUtils.h"
#include "../utils/OSMessageBox.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"
#include "GameSingletons.h"
#include "PersistenceUtils.h"

#include "dataloaders/UpgradesLoader.h"
#include "dataloaders/BaseGameDataLoader.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"

#include <rapidxml/rapidxml.hpp>
#include <fstream>
#include <sstream>

///------------------------------------------------------------------------------------------------

namespace persistence_utils
{

///------------------------------------------------------------------------------------------------

static const char* PROGRESS_SAVE_FILE_NAME = "progress_save";

///------------------------------------------------------------------------------------------------

bool ProgressSaveFileExists()
{
    return std::ifstream(objectiveC_utils::BuildLocalFileSaveLocation(PROGRESS_SAVE_FILE_NAME + std::string(".xml"))).good();
}

///------------------------------------------------------------------------------------------------

void LoadFromProgressSaveFile()
{
    class ProgressLoader: public BaseGameDataLoader
    {
    public:
        bool mCorruptedSaveFlag = false;
        
        void Load()
        {
            BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Seed"), [&](const void* n)
            {
                auto* node = static_cast<const rapidxml::xml_node<>*>(n);
                
                auto* seedValue = node->first_attribute("value");
                if (seedValue)
                {
                    GameSingletons::SetMapGenerationSeed(std::stoi(seedValue->value()));
                    GameSingletons::SetBackgroundIndex(GameSingletons::GetMapGenerationSeed() % game_constants::BACKGROUND_COUNT);
                    
                    if (GameSingletons::GetMapGenerationSeed() == 0)
                    {
                        mCorruptedSaveFlag = true;
                    }
                }
            });
            
            BaseGameDataLoader::SetCallbackForNode(strutils::StringId("CurrentMapCoord"), [&](const void* n)
            {
                auto* node = static_cast<const rapidxml::xml_node<>*>(n);
                
                auto* colValue = node->first_attribute("col");
                if (colValue)
                {
                    GameSingletons::SetCurrentMapCoord(MapCoord(std::stoi(colValue->value()), GameSingletons::GetCurrentMapCoord().mRow));
                }
                
                auto* rowValue = node->first_attribute("row");
                if (rowValue)
                {
                    GameSingletons::SetCurrentMapCoord(MapCoord(GameSingletons::GetCurrentMapCoord().mCol, std::stoi(rowValue->value())));
                }
            });
            
            BaseGameDataLoader::SetCallbackForNode(strutils::StringId("MapLevel"), [&](const void* n)
            {
                auto* node = static_cast<const rapidxml::xml_node<>*>(n);
                
                auto* mapLevel = node->first_attribute("level");
                if (mapLevel)
                {
                    GameSingletons::SetMapLevel(std::stoi(mapLevel->value()));
                }
            });
            
            BaseGameDataLoader::SetCallbackForNode(strutils::StringId("PlayerData"), [&](const void* n)
            {
                auto* node = static_cast<const rapidxml::xml_node<>*>(n);
                
                auto* maxHealth = node->first_attribute("maxHealth");
                if (maxHealth)
                {
                    GameSingletons::SetPlayerMaxHealth(std::stof(maxHealth->value()));
                }
                
                auto* health = node->first_attribute("health");
                if (health)
                {
                    GameSingletons::SetPlayerCurrentHealth(std::stof(health->value()));
                    GameSingletons::SetPlayerDisplayedHealth(std::stof(health->value()));
                }
                
                auto* attack = node->first_attribute("attack");
                if (attack)
                {
                    GameSingletons::SetPlayerAttackStat(std::stof(attack->value()));
                }
                
                auto* movement = node->first_attribute("movement");
                if (movement)
                {
                    GameSingletons::SetPlayerMovementSpeedStat(std::stof(movement->value()));
                }
                
                auto* bulletSpeed = node->first_attribute("bulletSpeed");
                if (bulletSpeed)
                {
                    GameSingletons::SetPlayerBulletSpeedStat(std::stof(bulletSpeed->value()));
                }
                
                auto* crystals = node->first_attribute("crystals");
                if (crystals)
                {
                    GameSingletons::SetCrystalCount(std::stof(crystals->value()));
                }
            });
            
            BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Upgrade"), [&](const void* n)
            {
                auto* node = static_cast<const rapidxml::xml_node<>*>(n);
                
                auto* upgradeName = node->first_attribute("name");
                if (upgradeName)
                {
                    auto upgradeNameId = strutils::StringId(upgradeName->value());
                    auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
                    auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
                    
                    const auto availableUpgradeIter = std::find_if(availableUpgrades.begin(), availableUpgrades.end(), [&](const UpgradeDefinition& upgradeDefinition){ return upgradeDefinition.mUpgradeNameId == upgradeNameId; });
                    
                    assert(availableUpgradeIter != availableUpgrades.cend());
                    const auto& upgradeDefinition = *availableUpgradeIter;
                    
                    equippedUpgrades.push_back(upgradeDefinition);
                    
                    if (upgradeDefinition.mIntransient == false)
                    {
                        availableUpgrades.erase(availableUpgradeIter);
                    }
                    
                    if (upgradeNameId == game_constants::PLAYER_SHIELD_UPGRADE_NAME)
                    {
                        GameSingletons::SetPlayerShieldHealth(std::stof(node->first_attribute("shieldHealth")->value()));
                    }
                }
            });
            
            BaseGameDataLoader::SetCallbackForNode(strutils::StringId("AvailableUpgrade"), [&](const void* n)
            {
                auto* node = static_cast<const rapidxml::xml_node<>*>(n);
                
                auto* upgradeName = node->first_attribute("name");
                if (upgradeName)
                {
                    auto upgradeNameId = strutils::StringId(upgradeName->value());
                    
                    auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
                    
                    const auto availableUpgradeIter = std::find_if(availableUpgrades.begin(), availableUpgrades.end(), [&](const UpgradeDefinition& upgradeDefinition){ return upgradeDefinition.mUpgradeNameId == upgradeNameId; });
                    
                    assert(availableUpgradeIter != availableUpgrades.cend());
                    auto& upgradeDefinition = *availableUpgradeIter;
                    
                    upgradeDefinition.mUnlockCost = std::stoi(node->first_attribute("unlockCost")->value());
                }
            });
            
            BaseGameDataLoader::LoadData(objectiveC_utils::BuildLocalFileSaveLocation(PROGRESS_SAVE_FILE_NAME));
        }
    };
    
    ProgressLoader pl;
    pl.Load();
    
    if (pl.mCorruptedSaveFlag)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::WARNING, "Corrupted Save File", "Found corrupted save file with seed " + std::to_string(GameSingletons::GetMapGenerationSeed()) + ". Cleaning up persistent files.");
        GenerateNewProgressSaveFile();
    }
}

///------------------------------------------------------------------------------------------------

void GenerateNewProgressSaveFile()
{
    auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
    auto& playerDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
    
    UpgradesLoader loader;
    GameSingletons::SetAvailableUpgrades(loader.LoadAllUpgrades());
    
    GameSingletons::GetEquippedUpgrades().clear();
    GameSingletons::SetMapGenerationSeed(math::RandomInt());
    GameSingletons::SetPlayerDisplayedHealth(playerDef.mHealth);
    GameSingletons::SetPlayerMaxHealth(playerDef.mHealth);
    GameSingletons::SetPlayerCurrentHealth(playerDef.mHealth);
    GameSingletons::SetPlayerAttackStat(playerDef.mDamage);
    GameSingletons::SetPlayerShieldHealth(0);
    GameSingletons::SetPlayerMovementSpeedStat(1.0f);
    GameSingletons::SetPlayerBulletSpeedStat(1.0f);
    GameSingletons::SetCrystalCount(0);
    GameSingletons::SetDisplayedCrystalCount(GameSingletons::GetCrystalCount());
    GameSingletons::SetCurrentMapCoord(MapCoord(game_constants::DEFAULT_MAP_COORD_COL, game_constants::DEFAULT_MAP_COORD_ROW));
    GameSingletons::SetMapLevel(0);
    GameSingletons::SetBackgroundIndex(GameSingletons::GetMapGenerationSeed() % game_constants::BACKGROUND_COUNT);
    
    BuildProgressSaveFile();
}

///------------------------------------------------------------------------------------------------

void BuildProgressSaveFile()
{
    std::ofstream progressSaveFile(objectiveC_utils::BuildLocalFileSaveLocation(PROGRESS_SAVE_FILE_NAME + std::string(".xml")));
    
    std::stringstream progressSaveFileXml;
    progressSaveFileXml << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
    progressSaveFileXml << "\n<SaveData>";
    progressSaveFileXml << "\n    <Seed value=\"" << GameSingletons::GetMapGenerationSeed() << "\" />";
    progressSaveFileXml << "\n    <CurrentMapCoord col=\"" << GameSingletons::GetCurrentMapCoord().mCol << "\" row=\"" << GameSingletons::GetCurrentMapCoord().mRow << "\" />";
    progressSaveFileXml << "\n    <MapLevel level=\"" << GameSingletons::GetMapLevel() << "\" />";
    progressSaveFileXml << "\n    <PlayerData maxHealth=\"" << std::to_string(GameSingletons::GetPlayerMaxHealth()) << "\" ";
    progressSaveFileXml << "health=\"" << std::to_string(GameSingletons::GetPlayerCurrentHealth()) << "\" ";
    progressSaveFileXml << "attack=\"" << std::to_string(GameSingletons::GetPlayerAttackStat()) << "\" ";
    progressSaveFileXml << "movement=\"" << std::to_string(GameSingletons::GetPlayerMovementSpeedStat()) << "\" ";
    progressSaveFileXml << "bulletSpeed=\"" << std::to_string(GameSingletons::GetPlayerBulletSpeedStat()) << "\" ";
    progressSaveFileXml << "crystals=\"" << GameSingletons::GetCrystalCount() << "\" ";
    progressSaveFileXml << "/>";
    
    progressSaveFileXml << "\n    <EquippedUpgrades>";
    for (const auto& equippedUpgrade: GameSingletons::GetEquippedUpgrades())
    {
        progressSaveFileXml << "\n        <Upgrade name=\"" << equippedUpgrade.mUpgradeNameId.GetString() << "\"";
        if (equippedUpgrade.mUpgradeNameId == game_constants::PLAYER_SHIELD_UPGRADE_NAME)
        {
            progressSaveFileXml << " shieldHealth=\"" << std::to_string(GameSingletons::GetPlayerShieldHealth()) << "\"";
        }
        progressSaveFileXml << " />";
    }
    
    progressSaveFileXml << "\n    </EquippedUpgrades>";
    
    progressSaveFileXml << "\n    <AvailableUpgrades>";
    for (const auto& availableUpgrade: GameSingletons::GetAvailableUpgrades())
    {
        progressSaveFileXml << "\n        <AvailableUpgrade name=\"" << availableUpgrade.mUpgradeNameId.GetString() << "\" ";
        progressSaveFileXml << " unlockCost=\"" << availableUpgrade.mUnlockCost << "\" ";
        progressSaveFileXml << " />";
    }
    
    progressSaveFileXml << "\n    </AvailableUpgrades>";
    
    progressSaveFileXml << "\n</SaveData>";
    
    const auto& progressSaveFileContents = progressSaveFileXml.str();
    progressSaveFile << progressSaveFileContents;
    progressSaveFile.close();
}

///------------------------------------------------------------------------------------------------

}
