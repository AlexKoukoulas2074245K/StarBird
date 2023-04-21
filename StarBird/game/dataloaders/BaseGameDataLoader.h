///------------------------------------------------------------------------------------------------
///  BaseGameDataLoader.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BaseGameDataLoader_h
#define BaseGameDataLoader_h

///------------------------------------------------------------------------------------------------

#include "../../utils/StringUtils.h"

#include <string>
#include <functional>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class BaseGameDataLoader
{
public:
    using NodeCallbackType = std::function<void(const void*)>;
    using NodeNameToCallbackType = std::unordered_map<strutils::StringId, NodeCallbackType, strutils::StringIdHasher>;
    
protected:
    BaseGameDataLoader() = default;
    
    void SetCallbackForNode(const strutils::StringId& nodeName, NodeCallbackType callback);
    void LoadData(const std::string& dataFileName, bool printOutDataFile = false);
    
private:
    NodeNameToCallbackType mNodeNameToCallbackMap;
};

///------------------------------------------------------------------------------------------------

#endif /* BaseGameDataLoader_h */
