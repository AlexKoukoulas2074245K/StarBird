///------------------------------------------------------------------------------------------------
///  TextureResource.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef TextureResource_h
#define TextureResource_h

///------------------------------------------------------------------------------------------------

#include "IResource.h"
#include "../utils/MathUtils.h"

#include <SDL_stdinc.h>
#include <SDL_surface.h>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

using GLuint = unsigned int;

///------------------------------------------------------------------------------------------------

class TextureResource final: public IResource
{
    friend class TextureLoader;

public:
    ~TextureResource();
    
    GLuint GetGLTextureId() const;
    const glm::vec2& GetDimensions() const;
    
private:
    TextureResource
    (
        SDL_Surface* const surface,
        const int width, 
        const int height,
        const int mode,
        const int format,
        GLuint glTextureId
    );
    
private:
    SDL_Surface* mSurface;
    glm::vec2 mDimensions;
    int mMode;
    int mFormat;
    GLuint mGLTextureId;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* TextureResource_h */
