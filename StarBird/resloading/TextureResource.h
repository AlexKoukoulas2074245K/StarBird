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

#include <memory>
#include <SDL_stdinc.h>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

using GLuint = unsigned int;

///------------------------------------------------------------------------------------------------

struct SheetElementMetadata
{
    float minU;
    float minV;
    float maxU;
    float maxV;
};

///------------------------------------------------------------------------------------------------

struct SheetRowMetadata
{
    std::vector<SheetElementMetadata> mColMetadata;
};

///------------------------------------------------------------------------------------------------

struct SheetMetadata
{
    std::vector<SheetRowMetadata> mRowMetadata;
};

///------------------------------------------------------------------------------------------------

class TextureResource final: public IResource
{
    friend class TextureLoader;

public:
    ~TextureResource();
    
    GLuint GetGLTextureId() const;
    glm::vec2 GetDimensions() const;
    glm::vec2 GetSingleTextureFrameDimensions() const;
    
    SheetMetadata* GetSheetMetadata() const;
    
private:
    TextureResource
    (
        const int width, 
        const int height,
        const int mode,
        const int format,
        GLuint glTextureId,
        std::unique_ptr<SheetMetadata> sheetMetadata
    );
    
private:
    glm::vec2 mDimensions;
    int mMode;
    int mFormat;
    GLuint mGLTextureId;
    std::unique_ptr<SheetMetadata> mSheetMetadata;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* TextureResource_h */
