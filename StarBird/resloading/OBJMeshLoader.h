///------------------------------------------------------------------------------------------------
///  OBJMeshLoader.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef OBJMeshLoader_h
#define OBJMeshLoader_h

///------------------------------------------------------------------------------------------------

#include "IResourceLoader.h"

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

class OBJMeshLoader final: public IResourceLoader
{
    friend class ResourceLoadingService;
    
public:
    void VInitialize() override;
    std::unique_ptr<IResource> VCreateAndLoadResource(const std::string& path) const override;
    
private:
    OBJMeshLoader() = default;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* OBJMeshLoader_h */
