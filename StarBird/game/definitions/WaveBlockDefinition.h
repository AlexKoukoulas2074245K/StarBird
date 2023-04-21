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

class WaveBlockLine
{
public:
    float GetLineHeight() const
    {
        return mEnemies.empty() ? 0.0f : mEnemies.back().mPosition.y - 20.0f + 2.0f;
    }
    
public:
    std::vector<WaveBlockEnemy> mEnemies;
};

///------------------------------------------------------------------------------------------------

class WaveBlockDefinition
{
public:
    void AdjustForDifficulty(const int difficulty)
    {
        if (difficulty == mDifficulty) return;
        
        // Find largest y in block line enemies
        auto waveHeight = 0.0f;
        auto riter = mWaveBlockLines.rbegin();
        while (riter != mWaveBlockLines.rend())
        {
            if (!riter->mEnemies.empty())
            {
                waveHeight = riter->mEnemies.back().mPosition.y - 20.0f;
                break;
            }
            
            riter++;
        }
        
        auto currentY = 20.0f + waveHeight + 2.0f;
        
        std::vector<WaveBlockLine> additionalLines;
        auto difficultyDiff = difficulty - mDifficulty;
        for (int i = 0; i < difficultyDiff; ++i)
        {
            auto targetLineCopy = mWaveBlockLines.at(i % mWaveBlockLines.size());
            auto lineHeight = targetLineCopy.GetLineHeight();
            
            for (auto& enemy: targetLineCopy.mEnemies)
            {
                enemy.mPosition.y = currentY + enemy.mPosition.y - 20.0f;
            }
            
            currentY += lineHeight;
            
            additionalLines.push_back(targetLineCopy);
        }
        
        mWaveBlockLines.insert(mWaveBlockLines.end(), additionalLines.begin(), additionalLines.end());
    }
    
public:
    std::vector<WaveBlockLine> mWaveBlockLines;
    strutils::StringId mBossName = strutils::StringId();
    float mBossHealth = 0.0f;
    int mDifficulty = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* WaveBlockDefinition_h */
