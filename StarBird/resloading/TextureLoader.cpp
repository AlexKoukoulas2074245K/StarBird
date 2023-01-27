///------------------------------------------------------------------------------------------------
///  TextureLoader.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#include "TextureLoader.h"
#include "TextureResource.h"
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
    
    //Convert surface to display format
    auto* pixels = SDL_ConvertSurfaceFormat( loadedSurface, SDL_GetWindowPixelFormat(SDL_GL_GetCurrentWindow()), 0 );
    if( pixels == NULL )
    {
        //printf( "Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError() );
    }
    
    //Color key image
    SDL_SetColorKey(pixels, SDL_TRUE, SDL_MapRGB(pixels->format, 0, 0xFF, 0xFF));
    
    //Create texture from surface pixels
    GLuint glTextureId;
    GL_CALL(glGenTextures(1, &glTextureId));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, glTextureId));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixels->w, pixels->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels->pixels));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    
    const auto surfaceWidth = loadedSurface->w;
    const auto surfaceHeight = loadedSurface->h;
    
    return std::unique_ptr<IResource>(new TextureResource(loadedSurface, surfaceWidth, surfaceHeight, GL_RGBA, GL_RGBA, glTextureId));
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
