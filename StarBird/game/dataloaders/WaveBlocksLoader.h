///------------------------------------------------------------------------------------------------
///  WaveBlocksLoader.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 21/04/2023
///------------------------------------------------------------------------------------------------

#ifndef WaveBlocksLoader_h
#define WaveBlocksLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "../definitions/WaveBlockDefinition.h"

#include <string>
#include <vector>

///------------------------------------------------------------------------------------------------

class WaveBlocksLoader: public BaseGameDataLoader
{
    friend class WaveBlocksRepository;
    
public:
    WaveBlocksLoader();
    
private:
    std::vector<WaveBlockDefinition>& LoadAllWaveBlocks();
    
private:
    std::vector<WaveBlockDefinition> mWaveBlocks;
};

///------------------------------------------------------------------------------------------------

#endif /* WaveBlocksLoader_h */
