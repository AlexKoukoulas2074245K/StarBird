///------------------------------------------------------------------------------------------------
///  DataFileLoader.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///-----------------------------------------------------------------------------------------------

#ifndef DataFileLoader_h
#define DataFileLoader_h

///-----------------------------------------------------------------------------------------------

#include "IResourceLoader.h"

///-----------------------------------------------------------------------------------------------

namespace resources
{

///-----------------------------------------------------------------------------------------------

class DataFileLoader final: public IResourceLoader
{
    friend class ResourceLoadingService;

public:
    void VInitialize() override;
    std::unique_ptr<IResource> VCreateAndLoadResource(const std::string& path) const override;
    
private:
    DataFileLoader() = default;

};

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------

#endif /* DataFileLoader_h */
