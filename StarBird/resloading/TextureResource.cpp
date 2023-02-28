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
}

///------------------------------------------------------------------------------------------------

GLuint TextureResource::GetGLTextureId() const
{
    return mGLTextureId;
}

///------------------------------------------------------------------------------------------------

glm::vec2 TextureResource::GetDimensions() const
{
    return mDimensions;
}

///------------------------------------------------------------------------------------------------

glm::vec2 TextureResource::GetSingleTextureFrameDimensions() const
{
    if (GetSheetMetadata())
    {
        const auto& firstColData = GetSheetMetadata()->mRowMetadata[0].mColMetadata[0];
        return glm::vec2((firstColData.maxU - firstColData.minU) * mDimensions.x, (firstColData.maxV - firstColData.minV) * mDimensions.y);
    }
    else
    {
        return GetDimensions();
    }
}

///------------------------------------------------------------------------------------------------

SheetMetadata* TextureResource::GetSheetMetadata() const
{
    if (mSheetMetadata)
        return mSheetMetadata.get();
    else
        return nullptr;
}

///------------------------------------------------------------------------------------------------

TextureResource::TextureResource
(
    const int width,
    const int height,
    const int mode,
    const int format,
    GLuint glTextureId,
    std::unique_ptr<SheetMetadata> sheetMetadata
)
    : mDimensions(width, height)
    , mMode(mode)
    , mFormat(format)
    , mGLTextureId(glTextureId)
    , mSheetMetadata(std::move(sheetMetadata))
{
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

