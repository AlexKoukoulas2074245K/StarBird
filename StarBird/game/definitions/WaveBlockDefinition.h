///------------------------------------------------------------------------------------------------
///  WaveBlockDefinition.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 21/04/2023
///------------------------------------------------------------------------------------------------

#ifndef WaveBlockDefinition_h
#define WaveBlockDefinition_h

///------------------------------------------------------------------------------------------------

#include "../../utils/MathUtils.h"
#include "../../utils/StringUtils.h"

#include <vector>

///------------------------------------------------------------------------------------------------

struct WaveBlockEnemy
{
    strutils::StringId mGameObjectEnemyType = strutils::StringId();
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
};

///------------------------------------------------------------------------------------------------

struct WaveBlockLine
{
    std::vector<WaveBlockEnemy> mEnemies;
};

///------------------------------------------------------------------------------------------------

struct WaveBlockDefinition
{
    std::vector<WaveBlockLine> mWaveBlockLines;
    strutils::StringId mBossName = strutils::StringId();
    float mBossHealth = 0.0f;
    int mDifficulty = 0;
    bool mInflexible = true;
    bool mExtensible = false;
};

///------------------------------------------------------------------------------------------------

#endif /* WaveBlockDefinition_h */
