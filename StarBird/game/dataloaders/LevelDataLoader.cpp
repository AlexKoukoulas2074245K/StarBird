///------------------------------------------------------------------------------------------------
///  LevelDataLoader.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "LevelDataLoader.h"
#include "../../utils/Logging.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

LevelDataLoader::LevelDataLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Camera"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        LevelCamera camera;
        
        auto* type = node->first_attribute("type");
        if (type)
        {
            camera.mType = strutils::StringId(type->value());
        }
        
        auto* lenseHeight = node->first_attribute("lenseHeight");
        if (lenseHeight)
        {
            camera.mLenseHeight = std::stof(lenseHeight->value());
        }
        
        mConstructedLevel.mCameras.push_back(camera);
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Wave"), [&](const void* n)
    {
        mConstructedLevel.mWaves.emplace_back();
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Enemy"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        LevelEnemy enemy;
        
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
            mConstructedLevel.mEnemyTypes.insert(enemy.mGameObjectEnemyType);
        }
        
        mConstructedLevel.mWaves.back().mEnemies.push_back(enemy);
    });
}

///------------------------------------------------------------------------------------------------

LevelDefinition& LevelDataLoader::LoadLevel(const std::string &levelName)
{
    mConstructedLevel = LevelDefinition();
    mConstructedLevel.mLevelName = strutils::StringId(levelName);
    
    BaseGameDataLoader::LoadData(levelName);
    
    return mConstructedLevel;
}

///------------------------------------------------------------------------------------------------
