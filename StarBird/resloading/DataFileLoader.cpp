///------------------------------------------------------------------------------------------------
///  DataFileLoader.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///-----------------------------------------------------------------------------------------------

#include "DataFileLoader.h"
#include "DataFileResource.h"
#include "../utils/OSMessageBox.h"
#include "../utils/StringUtils.h"

#include <fstream>   // ifstream
#include <streambuf> // istreambuf_iterator

///-----------------------------------------------------------------------------------------------

namespace resources
{

///-----------------------------------------------------------------------------------------------
void DataFileLoader::VInitialize()
{ 
}

///-----------------------------------------------------------------------------------------------

std::unique_ptr<IResource> DataFileLoader::VCreateAndLoadResource(const std::string& resourcePath) const
{
    std::ifstream file(resourcePath);
    
    if (!file.good())
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "File could not be found", resourcePath.c_str());
        return nullptr;
    }
    
    std::string str;
    
    file.seekg(0, std::ios::end);
    str.reserve(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);
    
    str.assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());
    
    return std::unique_ptr<IResource>(new DataFileResource(str));
}

///-----------------------------------------------------------------------------------------------

}

