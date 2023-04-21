///------------------------------------------------------------------------------------------------
///  WaveBlocksRepository.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 21/04/2023
///------------------------------------------------------------------------------------------------

#ifndef WaveBlocksRepository_h
#define WaveBlocksRepository_h

///------------------------------------------------------------------------------------------------

#include "../dataloaders/WaveBlocksLoader.h"
#include "../definitions/WaveBlockDefinition.h"
#include "../../utils/StringUtils.h"

#include <optional>
#include <vector>

///------------------------------------------------------------------------------------------------

class WaveBlocksRepository final
{
public:
    static WaveBlocksRepository& GetInstance();
    
    WaveBlocksRepository(const WaveBlocksRepository&) = delete;
    WaveBlocksRepository(WaveBlocksRepository&&) = delete;
    const WaveBlocksRepository& operator = (const WaveBlocksRepository&) = delete;
    WaveBlocksRepository& operator = (WaveBlocksRepository&&) = delete;
    
    std::vector<WaveBlockDefinition> GetEligibleWaveBlocksForDifficulty(int difficultyValue) const;
    WaveBlockDefinition GetBossWaveBlock(const strutils::StringId& bossName) const;
    
    void LoadWaveBlocks();
    
private:
    WaveBlocksRepository() = default;
    
private:
    WaveBlocksLoader mLoader;
    std::vector<WaveBlockDefinition> mWaveBlocks;
};

///------------------------------------------------------------------------------------------------

#endif /* WaveBlocksRepository_h */
