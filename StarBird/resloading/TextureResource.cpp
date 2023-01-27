///------------------------------------------------------------------------------------------------
///  TextureResource.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#include "TextureResource.h"
#include "../utils/OpenGL.h"

#include <cassert>
#include <SDL_pixels.h>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

TextureResource::~TextureResource()
{
    GL_CALL(glDeleteTextures(1, &mGLTextureId));
    if (mSurface) SDL_FreeSurface(mSurface);
}

///------------------------------------------------------------------------------------------------

GLuint TextureResource::GetGLTextureId() const
{
    return mGLTextureId;
}

///------------------------------------------------------------------------------------------------

const glm::vec2& TextureResource::GetDimensions() const
{
    return mDimensions;
}

///------------------------------------------------------------------------------------------------

TextureResource::TextureResource
(
    SDL_Surface* const surface,
    const int width,
    const int height,
    const int mode,
    const int format,
    GLuint glTextureId
)
    : mSurface(surface)
    , mDimensions(width, height)
    , mMode(mode)
    , mFormat(format)
    , mGLTextureId(glTextureId)
{
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

