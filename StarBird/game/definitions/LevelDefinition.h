///------------------------------------------------------------------------------------------------
///  LevelDefinition.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LevelDefinition_h
#define LevelDefinition_h

///------------------------------------------------------------------------------------------------

#include "../../utils/MathUtils.h"
#include "../../utils/StringUtils.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

struct LevelEnemy
{
    strutils::StringId mGameObjectEnemyType = strutils::StringId();
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
};

///------------------------------------------------------------------------------------------------

struct LevelCamera
{
    strutils::StringId mType = strutils::StringId();
    float mLenseHeight = 0.0f;
};

///------------------------------------------------------------------------------------------------

struct LevelWave
{
    strutils::StringId mBossName = strutils::StringId();
    float mBossHealth = 0.0f;
    std::vector<LevelEnemy> mEnemies;
};

///------------------------------------------------------------------------------------------------

struct LevelDefinition
{
    strutils::StringId mLevelName = strutils::StringId();
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mEnemyTypes;
    std::vector<LevelWave> mWaves;
    std::vector<LevelCamera> mCameras;
};

///------------------------------------------------------------------------------------------------

#endif /* LevelDefinition_h */
