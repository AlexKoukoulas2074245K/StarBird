///------------------------------------------------------------------------------------------------
///  WaveBlocksLoader.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 21/04/2023
///------------------------------------------------------------------------------------------------

#include "WaveBlocksLoader.h"
#include "../../utils/Logging.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

WaveBlocksLoader::WaveBlocksLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("WaveBlock"), [&](const void* n)
    {
        mWaveBlocks.emplace_back();
        
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        if (node->first_attribute("bossName"))
        {
            mWaveBlocks.back().mBossName = strutils::StringId(node->first_attribute("bossName")->value());
        }
        
        if (node->first_attribute("bossHealth"))
        {
            mWaveBlocks.back().mBossHealth = std::stof(node->first_attribute("bossHealth")->value());
        }
        
        if (node->first_attribute("difficulty"))
        {
            mWaveBlocks.back().mDifficulty = std::stoi(node->first_attribute("difficulty")->value());
        }
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("WaveBlockLine"), [&](const void* n)
    {
        mWaveBlocks.back().mWaveBlockLines.emplace_back();
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Enemy"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        WaveBlockEnemy enemy;
        
        auto* position = node->first_attribute("position");
        if (position)
        {
            auto positionComponents = strutils::StringSplit(std::string(position->value()), ',');
            enemy.mPosition.x = std::stof(positionComponents[0]);
            enemy.mPosition.y = std::stof(positionComponents[1]);
        }
        
        auto* enemyType = node->first_attribute("type");
        if (enemyType)
        {
            enemy.mGameObjectEnemyType = strutils::StringId(enemyType->value());
        }
        
        mWaveBlocks.back().mWaveBlockLines.back().mEnemies.push_back(enemy);
    });
}

///------------------------------------------------------------------------------------------------

std::vector<WaveBlockDefinition> WaveBlocksLoader::LoadAllWaveBlocks()
{
    BaseGameDataLoader::LoadData("wave_blocks");
    
    return mWaveBlocks;
}

///------------------------------------------------------------------------------------------------
