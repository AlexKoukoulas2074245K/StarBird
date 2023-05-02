///------------------------------------------------------------------------------------------------
///  LevelGeneration.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/05/2023                                                       
///------------------------------------------------------------------------------------------------

#include "GameSingletons.h"
#include "LevelGeneration.h"
#include "datarepos/WaveBlocksRepository.h"
#include "definitions/WaveBlockDefinition.h"
#include "../utils/Logging.h"
#include "../utils/ObjectiveCUtils.h"

#include <fstream>

///------------------------------------------------------------------------------------------------

static const float LEVEL_WAVE_Y_INCREMENT = 2.0f;

///------------------------------------------------------------------------------------------------

namespace level_generation
{

///------------------------------------------------------------------------------------------------

static void ExtendWaveBlockForDifficulty(const int difficulty, WaveBlockDefinition& waveBlock);
static float GetWaveBlockLineHeight(const WaveBlockLine& waveBlockLine);

///------------------------------------------------------------------------------------------------

void GenerateLevel(const MapCoord& mapCoord, const Map::NodeData& nodeData)
{
    Log(LogType::INFO, "Generating level for map node %s", mapCoord.ToString().c_str());
    
    std::stringstream levelXml;
    levelXml <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "\n<Level>"
    "\n<Camera type=\"world_cam\" lenseHeight=\"30.0f\"/>"
    "\n<Camera type=\"gui_cam\" lenseHeight=\"30.0f\"/>";
    
    
    auto difficultyValue = mapCoord.mCol;
    if (nodeData.mNodeType == Map::NodeType::BOSS_ENCOUNTER)
    {
        difficultyValue *= 1.5f;
    }
    else if (nodeData.mNodeType == Map::NodeType::HARD_ENCOUNTER)
    {
        difficultyValue *= 2.0f;
    }
    
    difficultyValue += GameSingletons::GetMapLevel() * 10;
    
    auto waveCount = math::ControlledRandomInt(2,3) + (difficultyValue / 5);
    
    const auto& eligibleBlocks = WaveBlocksRepository::GetInstance().GetEligibleWaveBlocksForDifficulty(difficultyValue);
    
    if (eligibleBlocks.size() == 0) return;
    
    for (int j = 0; j < waveCount; ++j)
    {
        levelXml << "\n    <Wave";
        auto selectedBlockIndex = math::ControlledRandomInt(0, static_cast<int>(eligibleBlocks.size()) - 1);
        auto selectedBlock = eligibleBlocks.at(selectedBlockIndex);
        if (selectedBlock.mExtensible)
        {
            ExtendWaveBlockForDifficulty(difficultyValue, selectedBlock);
        }
        
        levelXml << " blockIndex=\"" << std::to_string(selectedBlockIndex) << "\" difficulty=\"" << std::to_string(difficultyValue) << "\"";
        
        if (nodeData.mNodeType == Map::NodeType::BOSS_ENCOUNTER && j == waveCount - 1)
        {
            selectedBlock = WaveBlocksRepository::GetInstance().GetBossWaveBlock(strutils::StringId("Ka'thun"));
            levelXml << " bossName=\"" << selectedBlock.mBossName.GetString() << "\" bossHealth=\"" << selectedBlock.mBossHealth << "\"";
        }
        
        levelXml << ">";
        for (const auto& blockLine: selectedBlock.mWaveBlockLines)
        {
            for (const auto& enemy: blockLine.mEnemies)
            {
                auto positionOffset = selectedBlock.mInflexible ? glm::vec2(0.0f, 0.0f) : glm::vec2(math::RandomFloat(-1.0f, 1.0f), math::RandomFloat(-1.0f, 1.0f));
                levelXml << "\n        <Enemy position=\"" << enemy.mPosition.x + positionOffset.x << ", " << enemy.mPosition.y + positionOffset.y << "\" type=\"" << enemy.mGameObjectEnemyType.GetString() << "\"/>";
            }
        }
        
        levelXml << "\n    </Wave>";
    }
    
    levelXml << "\n</Level>";
    
    auto levelFileName = objectiveC_utils::BuildLocalFileSaveLocation(mapCoord.ToString() + ".xml");
    std::ofstream outputFile(levelFileName);
    outputFile << levelXml.str();
    outputFile.close();
}

///------------------------------------------------------------------------------------------------

void ExtendWaveBlockForDifficulty(const int difficulty, WaveBlockDefinition& waveBlock)
{
    if (difficulty == waveBlock.mDifficulty) return;
    
    // Find largest y in block line enemies
    auto waveHeight = 0.0f;
    auto riter = waveBlock.mWaveBlockLines.rbegin();
    while (riter != waveBlock.mWaveBlockLines.rend())
    {
        if (!riter->mEnemies.empty())
        {
            waveHeight = riter->mEnemies.back().mPosition.y - game_constants::LEVEL_WAVE_VISIBLE_Y;
            break;
        }
        
        riter++;
    }
    
    auto currentY = game_constants::LEVEL_WAVE_VISIBLE_Y + waveHeight + LEVEL_WAVE_Y_INCREMENT;
    
    std::vector<WaveBlockLine> additionalLines;
    auto difficultyDiff = difficulty - waveBlock.mDifficulty;
    for (int i = 0; i < difficultyDiff; ++i)
    {
        auto targetLineCopy = waveBlock.mWaveBlockLines.at(i % waveBlock.mWaveBlockLines.size());
        auto lineHeight = GetWaveBlockLineHeight(targetLineCopy);
        
        for (auto& enemy: targetLineCopy.mEnemies)
        {
            enemy.mPosition.y = currentY + enemy.mPosition.y - game_constants::LEVEL_WAVE_VISIBLE_Y;
        }
        
        if (!waveBlock.mInflexible)
        {
            currentY += lineHeight * math::Max(0.0f, (1.0f - difficultyDiff/20.0f));
        }
        else
        {
            currentY += lineHeight;
        }
        
        additionalLines.push_back(targetLineCopy);
    }
    
    waveBlock.mWaveBlockLines.insert(waveBlock.mWaveBlockLines.end(), additionalLines.begin(), additionalLines.end());
}

///------------------------------------------------------------------------------------------------

float GetWaveBlockLineHeight(const WaveBlockLine& waveBlockLine)
{
    return waveBlockLine.mEnemies.empty() ? 0.0f : waveBlockLine.mEnemies.back().mPosition.y - game_constants::LEVEL_WAVE_VISIBLE_Y + LEVEL_WAVE_Y_INCREMENT;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
