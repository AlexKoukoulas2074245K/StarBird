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
#include "../definitions/ObjectTypeDefinition.h"

#include <string>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinitionLoader final: public BaseGameDataLoader
{
public:
    friend class ObjectTypeDefinitionRepository;
    
    ObjectTypeDefinitionLoader();

private:
    ObjectTypeDefinition&& LoadObjectTypeDefinition(const std::string& objectTypeDefinitionFileName, std::unordered_set<strutils::StringId, strutils::StringIdHasher>* subObjectsFound);
    
private:
    ObjectTypeDefinition mConstructedObjectTypeDef;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher>* mSubObjectsFound;
};

///------------------------------------------------------------------------------------------------

#endif /* ObjectTypeDefinitionLoader_h */
