///------------------------------------------------------------------------------------------------
///  WaveBlocksRepository.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 21/04/2023
///------------------------------------------------------------------------------------------------

#include "WaveBlocksRepository.h"
#include "../../utils/OSMessageBox.h"

///------------------------------------------------------------------------------------------------

WaveBlocksRepository& WaveBlocksRepository::GetInstance()
{
    static WaveBlocksRepository instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

std::vector<WaveBlockDefinition> WaveBlocksRepository::GetEligibleWaveBlocksForDifficulty(int difficultyValue) const
{
    std::vector<WaveBlockDefinition> result;
    for (const auto& waveBlockDef: mWaveBlocks)
    {
        if (waveBlockDef.mDifficulty <= difficultyValue)
        {
            result.push_back(waveBlockDef);
        }
    }
    
    return result;
}

///------------------------------------------------------------------------------------------------

WaveBlockDefinition WaveBlocksRepository::GetBossWaveBlock(const strutils::StringId& bossName) const
{
    auto findIter = std::find_if(mWaveBlocks.begin(), mWaveBlocks.end(), [&](const WaveBlockDefinition& waveBlockDefinition)
    {
        return waveBlockDefinition.mBossName == bossName;
    });
    
    return findIter != mWaveBlocks.end() ? *findIter : WaveBlockDefinition();
}

///------------------------------------------------------------------------------------------------

void WaveBlocksRepository::LoadWaveBlocks()
{
    mWaveBlocks = mLoader.LoadAllWaveBlocks();
}

///------------------------------------------------------------------------------------------------
