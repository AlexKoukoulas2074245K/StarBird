///------------------------------------------------------------------------------------------------
///  FontDefinition.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/02/2023
///------------------------------------------------------------------------------------------------

#ifndef FontDefinition_h
#define FontDefinition_h

///------------------------------------------------------------------------------------------------

#include "../../utils/MathUtils.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/StringUtils.h"

#include <unordered_map>


///------------------------------------------------------------------------------------------------

struct Glyph
{
    float minU = 0.0f;
    float minV = 0.0f;
    float maxU = 0.0f;
    float maxV = 0.0f;
    float mYOffsetPixels = 0.0f;
    float mWidthPixels = 0.0f;
    float mHeightPixels = 0.0f;
    float mAdvancePixels = 0.0f;
};

///------------------------------------------------------------------------------------------------

struct FontDefinition
{
    strutils::StringId mFontName = strutils::StringId();
    resources::ResourceId mFontTextureResourceId;
    std::unordered_map<char, Glyph> mGlyphs;
    glm::vec2 mFontTextureDimensions = glm::vec2(0.0f, 0.0f);
};

///------------------------------------------------------------------------------------------------

#endif /* FontDefinition_h */
