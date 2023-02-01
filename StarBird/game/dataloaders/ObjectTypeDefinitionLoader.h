///------------------------------------------------------------------------------------------------
///  ObjectTypeDefinitionLoader.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ObjectTypeDefinitionLoader_h
#define ObjectTypeDefinitionLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "ObjectTypeDefinition.h"

#include <string>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinitionLoader: public BaseGameDataLoader
{
public:
    ObjectTypeDefinitionLoader();
    ObjectTypeDefinition& LoadObjectTypeDefinition(const std::string& objectTypeDefinitionFileName);

private:
    ObjectTypeDefinition mConstructedObjectTypeDef;
};

///------------------------------------------------------------------------------------------------

#endif /* ObjectTypeDefinitionLoader_h */
