///------------------------------------------------------------------------------------------------
///  LevelDataLoader.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LevelDataLoader_h
#define LevelDataLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "LevelDefinition.h"

#include <string>

///------------------------------------------------------------------------------------------------

class LevelDataLoader: public BaseGameDataLoader
{
public:
    LevelDataLoader();
    LevelDefinition& LoadLevel(const std::string& levelName);

private:
    LevelDefinition mConstructedLevel;
};

///------------------------------------------------------------------------------------------------

#endif /* LevelDataLoader_h */
