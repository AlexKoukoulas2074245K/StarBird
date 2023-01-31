///------------------------------------------------------------------------------------------------
///  GameObjectDefinitionLoader.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameObjectDefinitionLoader_h
#define GameObjectDefinitionLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "GameObjectDefinition.h"

#include <string>

///------------------------------------------------------------------------------------------------

class GameObjectDefinitionLoader: public BaseGameDataLoader
{
public:
    GameObjectDefinitionLoader();
    GameObjectDefinition& LoadGameObjectDefinition(const std::string& gameObjectDefinitionFileName);

private:
    GameObjectDefinition mConstructedGODef;
};

///------------------------------------------------------------------------------------------------

#endif /* GameObjectDefinitionLoader_h */
