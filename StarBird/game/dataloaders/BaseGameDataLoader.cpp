///------------------------------------------------------------------------------------------------
///  BaseGameDataLoader.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "../../resloading/DataFileResource.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/ObjectiveCUtils.h"
#include "../../utils/Logging.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

void ProcessNode(const rapidxml::xml_node<>* node, BaseGameDataLoader::NodeNameToCallbackType& callbackMap)
{
    auto findIter = callbackMap.find(strutils::StringId(node->name()));
    if (findIter != callbackMap.end())
    {
        findIter->second(node);
    }
}

///------------------------------------------------------------------------------------------------

void RecursivelyTraverseViewNodes(const rapidxml::xml_node<>* node, BaseGameDataLoader::NodeNameToCallbackType& callbackMap)
{
    if (node == nullptr || std::strlen(node->name()) == 0) return;
    
    ProcessNode(node, callbackMap);
    
    for (auto childNode = node->first_node(); childNode; childNode = childNode->next_sibling())
    {
        RecursivelyTraverseViewNodes(childNode, callbackMap);
    }
}

///------------------------------------------------------------------------------------------------

void BaseGameDataLoader::SetCallbackForNode(const strutils::StringId& nodeName, NodeCallbackType callback)
{
    mNodeNameToCallbackMap[nodeName] = callback;
}

///------------------------------------------------------------------------------------------------

void BaseGameDataLoader::LoadData(const std::string& dataFileName)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    auto resourceId = resService.LoadResource(strutils::StringStartsWith(dataFileName, objectiveC_utils::GetLocalFileSaveLocation()) ? (dataFileName + ".xml") : (resources::ResourceLoadingService::RES_DATA_ROOT + dataFileName + ".xml"));
    auto& xmlLevel = resService.GetResource<resources::DataFileResource>(resourceId);
    auto xmlContents = xmlLevel.GetContents();
    resService.UnloadResource(resourceId);
    
    rapidxml::xml_document<> doc;
    doc.parse<0>(&xmlContents[0]);
        
    auto node = doc.first_node();
    
    RecursivelyTraverseViewNodes(node, mNodeNameToCallbackMap);
}

///------------------------------------------------------------------------------------------------
