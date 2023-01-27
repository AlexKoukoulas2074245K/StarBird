///------------------------------------------------------------------------------------------------
///  TextureLoader.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef TextureLoader_h
#define TextureLoader_h

///------------------------------------------------------------------------------------------------

#include "IResourceLoader.h"

#include <memory>
#include <SDL_stdinc.h>
#include <set>

///------------------------------------------------------------------------------------------------

struct SDL_Surface;

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

class TextureLoader final: public IResourceLoader
{
    friend class ResourceLoadingService;

public:
    void VInitialize() override;
    std::unique_ptr<IResource> VCreateAndLoadResource(const std::string& path) const override;

private:
    TextureLoader() = default;
    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* TextureLoader_h */
