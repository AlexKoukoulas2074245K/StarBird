///------------------------------------------------------------------------------------------------
///  FontLoader.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/02/2023
///------------------------------------------------------------------------------------------------

#include "FontLoader.h"
#include "../../resloading/TextureResource.h"
#include "../../utils/Logging.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

FontLoader::FontLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("character"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        Glyph glyph;
        
        auto* width = node->first_attribute("width");
        if (width)
        {
            glyph.mWidthPixels = std::stoi(width->value());
        }
        
        auto* height = node->first_attribute("height");
        if (height)
        {
            glyph.mHeightPixels = std::stoi(height->value());
        }
        
        auto* x = node->first_attribute("x");
        if (x)
        {
            auto normalizedU = std::stof(x->value()) / mConstructedFont.mFontTextureDimensions.x;
            glyph.minU = normalizedU;
            glyph.maxU = normalizedU + glyph.mWidthPixels / mConstructedFont.mFontTextureDimensions.x;
        }
        
        auto* y = node->first_attribute("y");
        if (y)
        {
            auto normalizedV = (mConstructedFont.mFontTextureDimensions.y - std::stof(y->value())) / mConstructedFont.mFontTextureDimensions.y;
            glyph.minV = normalizedV - glyph.mHeightPixels / mConstructedFont.mFontTextureDimensions.y;
            glyph.maxV = normalizedV;
        }
        
        auto* yOffset = node->first_attribute("origin-y");
        if (yOffset)
        {
            glyph.mYOffsetPixels = std::stof(yOffset->value());
        }
        
        auto* advance = node->first_attribute("advance");
        if (advance)
        {
            glyph.mAdvancePixels = std::stof(advance->value());
        }
        
        auto* text = node->first_attribute("text");
        if (text)
        {
            // special cases
            if (strcmp(text->value(), "\"") == 0)
            {
                mConstructedFont.mGlyphs['"'] = std::move(glyph);
            }
            else if (strcmp(text->value(), "&") == 0)
            {
                mConstructedFont.mGlyphs['&'] = std::move(glyph);
            }
            else if (strcmp(text->value(), "<") == 0)
            {
                mConstructedFont.mGlyphs['<'] = std::move(glyph);
            }
            else if (strcmp(text->value(), ">") == 0)
            {
                mConstructedFont.mGlyphs['>'] = std::move(glyph);
            }
            else if (strcmp(text->value(), "'") == 0)
            {
                mConstructedFont.mGlyphs['\''] = std::move(glyph);
            }
            else
            {
                mConstructedFont.mGlyphs[text->value()[0]] = std::move(glyph);
            }
        }
    });
}

///------------------------------------------------------------------------------------------------

FontDefinition& FontLoader::LoadFont(const std::string &fontName)
{
    mConstructedFont = FontDefinition();
    mConstructedFont.mFontName = strutils::StringId(fontName);
    mConstructedFont.mFontTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + fontName + ".bmp");
    
    const auto& fontTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(mConstructedFont.mFontTextureResourceId);
    mConstructedFont.mFontTextureDimensions = fontTexture.GetDimensions();
   
    BaseGameDataLoader::LoadData(fontName);
    
    return mConstructedFont;
}

///------------------------------------------------------------------------------------------------
