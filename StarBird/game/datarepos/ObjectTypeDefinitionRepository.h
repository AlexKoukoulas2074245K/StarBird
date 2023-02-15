///------------------------------------------------------------------------------------------------
///  ObjectTypeDefinitionRepository.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ObjectTypeDefinitionRepository_h
#define ObjectTypeDefinitionRepository_h

///------------------------------------------------------------------------------------------------

#include "../dataloaders/ObjectTypeDefinitionLoader.h"
#include "../definitions/ObjectTypeDefinition.h"
#include "../../utils/StringUtils.h"

#include <optional>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class ObjectTypeDefinitionRepository final
{
public:
    static ObjectTypeDefinitionRepository& GetInstance();
    
    ~ObjectTypeDefinitionRepository() = default;
    ObjectTypeDefinitionRepository(const ObjectTypeDefinitionRepository&) = delete;
    ObjectTypeDefinitionRepository(ObjectTypeDefinitionRepository&&) = delete;
    const ObjectTypeDefinitionRepository& operator = (const ObjectTypeDefinitionRepository&) = delete;
    ObjectTypeDefinitionRepository& operator = (ObjectTypeDefinitionRepository&&) = delete;
    
    std::optional<std::reference_wrapper<const ObjectTypeDefinition>> GetObjectTypeDefinition(const strutils::StringId& objectTypeDefName) const;
    void LoadObjectTypeDefinition(const strutils::StringId& objectTypeDefName);
    
private:
    ObjectTypeDefinitionRepository() = default;
    
private:
    ObjectTypeDefinitionLoader mLoader;
    std::unordered_map<strutils::StringId, ObjectTypeDefinition, strutils::StringIdHasher> mObjectTypeDefinitionsMap;
};

///------------------------------------------------------------------------------------------------

#endif /* ObjectTypeDefinitionRepository_h */
