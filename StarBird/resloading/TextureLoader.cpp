///------------------------------------------------------------------------------------------------
///  TextureLoader.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#include "TextureLoader.h"
#include "TextureResource.h"
#include "../utils/FileUtils.h"
#include "../utils/Logging.h"
#include "../utils/OSMessageBox.h"
#include "../utils/StringUtils.h"
#include "../utils/OpenGL.h"

#include <algorithm>
#include <fstream>     // ifstream
#include <SDL.h>
#include <iostream>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

void TextureLoader::VInitialize()
{
}

///------------------------------------------------------------------------------------------------

std::unique_ptr<IResource> TextureLoader::VCreateAndLoadResource(const std::string& resourcePath) const
{
    std::ifstream file(resourcePath);
    
    if (!file.good())
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "File could not be found", resourcePath.c_str());
        return nullptr;
    }
    
    SDL_Surface* loadedSurface = SDL_LoadBMP(resourcePath.c_str());
    if( loadedSurface == NULL )
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Error Loading Texture", resourcePath.c_str());
        return nullptr;
    }
    
    // Convert surface to display format
    auto* pixels = SDL_ConvertSurfaceFormat( loadedSurface, SDL_GetWindowPixelFormat(SDL_GL_GetCurrentWindow()), 0 );
    
    // Color key image
    SDL_SetColorKey(pixels, SDL_TRUE, SDL_MapRGB(pixels->format, 0, 0xFF, 0xFF));
    
    bool useMipMap = strutils::StringEndsWith(fileutils::GetFileNameWithoutExtension(resourcePath), "mm");
    bool useUVWrap = !strutils::StringEndsWith(fileutils::GetFileNameWithoutExtension(resourcePath), "fx");
    
    // Create texture from surface pixels
    GLuint glTextureId;
    GL_CALL(glGenTextures(1, &glTextureId));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, glTextureId));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixels->w, pixels->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels->pixels));
    
    if (useMipMap)
    {
        GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    }
    else
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }
    
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    if (useUVWrap)
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    }
    else
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    }
    
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    
    const auto surfaceWidth = loadedSurface->w;
    const auto surfaceHeight = loadedSurface->h;
    
    SDL_FreeSurface(loadedSurface);
    
    // Check for spritesheet metadata
    std::unique_ptr<SheetMetadata> sheetMetadata = nullptr;
    auto metadataFilePath = resourcePath;
    strutils::StringReplaceAllOccurences("bmp", "mtd", metadataFilePath);
    std::ifstream metadataFile(metadataFilePath);
    
    if (metadataFile.good())
    {
        std::string line;
        sheetMetadata = std::make_unique<SheetMetadata>();
        
        float uvCounterX = 0.0f;
        float uvCounterY = 1.0f;
        
        while (std::getline(metadataFile, line))
        {
            sheetMetadata->mRowMetadata.emplace_back();
            auto& currentRow = sheetMetadata->mRowMetadata.back();
            
            auto splitByComma = strutils::StringSplit(line, ',');
            assert(splitByComma.size() == 3);
            auto elementNormalizedWidth =  std::stoi(splitByComma[0])/(float)surfaceWidth;
            auto elementNormalizedHeight = std::stoi(splitByComma[1])/(float)surfaceHeight;
            auto elementCount = std::stoi(splitByComma[2]);
            
            for (auto i = 0; i < elementCount; ++i)
            {
                currentRow.mColMetadata.emplace_back();
                auto& currentCol = currentRow.mColMetadata.back();
                
                currentCol.minU = uvCounterX;
                currentCol.minV = uvCounterY - elementNormalizedHeight;
                currentCol.maxU = uvCounterX + elementNormalizedWidth;
                currentCol.maxV = uvCounterY;
               
                uvCounterX += elementNormalizedWidth;
            }
            
            uvCounterX = 0.0f;
            uvCounterY -= elementNormalizedHeight;
        }
    }
    
    return std::unique_ptr<IResource>(new TextureResource(surfaceWidth, surfaceHeight, GL_RGBA, GL_RGBA, glTextureId, std::move(sheetMetadata)));
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
