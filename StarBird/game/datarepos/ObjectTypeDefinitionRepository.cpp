///------------------------------------------------------------------------------------------------
///  ObjectTypeDefinitionRepository.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "ObjectTypeDefinitionRepository.h"

///------------------------------------------------------------------------------------------------

ObjectTypeDefinitionRepository& ObjectTypeDefinitionRepository::GetInstance()
{
    static ObjectTypeDefinitionRepository instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<ObjectTypeDefinition>> ObjectTypeDefinitionRepository::GetMutableObjectTypeDefinition(const strutils::StringId& objectTypeDefName)
{
    auto findIter = mObjectTypeDefinitionsMap.find(objectTypeDefName);
    if (findIter != mObjectTypeDefinitionsMap.end())
    {
        return std::optional<std::reference_wrapper<ObjectTypeDefinition>>{findIter->second};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<const ObjectTypeDefinition>> ObjectTypeDefinitionRepository::GetObjectTypeDefinition(const strutils::StringId& objectTypeDefName) const
{
    auto findIter = mObjectTypeDefinitionsMap.find(objectTypeDefName);
    if (findIter != mObjectTypeDefinitionsMap.end())
    {
        return std::optional<std::reference_wrapper<const ObjectTypeDefinition>>{findIter->second};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void ObjectTypeDefinitionRepository::LoadObjectTypeDefinition(const strutils::StringId& objectTypeDefName)
{
    auto findIter = mObjectTypeDefinitionsMap.find(objectTypeDefName);
    if (findIter == mObjectTypeDefinitionsMap.end())
    {
        mObjectTypeDefinitionsMap[objectTypeDefName] = mLoader.LoadObjectTypeDefinition(objectTypeDefName.GetString());
    }
}

///------------------------------------------------------------------------------------------------

ObjectTypeDefinitionRepository::~ObjectTypeDefinitionRepository()
{
    for (auto& objectDef: mObjectTypeDefinitionsMap)
    {
        for (auto& animation: objectDef.second.mAnimations)
        {
            delete animation.second;
        }
    }
}


///------------------------------------------------------------------------------------------------
