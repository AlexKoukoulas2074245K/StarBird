///------------------------------------------------------------------------------------------------
///  ResourceLoadingService.cpp
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#include "ResourceLoadingService.h"
#include "DataFileLoader.h"
#include "IResource.h"
#include "OBJMeshLoader.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "../utils/FileUtils.h"
#include "../utils/Logging.h"
#include "../utils/OSMessageBox.h"
#include "../utils/StringUtils.h"
#include "../utils/TypeTraits.h"

#include <fstream>
#include <cassert>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

const std::string ResourceLoadingService::RES_ROOT               = "res/";
const std::string ResourceLoadingService::RES_DATA_ROOT          = RES_ROOT + "data/";
const std::string ResourceLoadingService::RES_SCRIPTS_ROOT       = RES_ROOT + "scripts/";
const std::string ResourceLoadingService::RES_MODELS_ROOT        = RES_ROOT + "models/";
const std::string ResourceLoadingService::RES_MUSIC_ROOT         = RES_ROOT + "music/";
const std::string ResourceLoadingService::RES_SFX_ROOT           = RES_ROOT + "sfx/";
const std::string ResourceLoadingService::RES_SHADERS_ROOT       = RES_ROOT + "shaders/";
const std::string ResourceLoadingService::RES_TEXTURES_ROOT      = RES_ROOT + "textures/";
const std::string ResourceLoadingService::RES_ATLASES_ROOT       = RES_TEXTURES_ROOT + "atlases/";
const std::string ResourceLoadingService::RES_FONT_MAP_DATA_ROOT = RES_DATA_ROOT + "font_maps/";

const ResourceId ResourceLoadingService::FALLBACK_TEXTURE_ID = 0;
const ResourceId ResourceLoadingService::FALLBACK_SHADER_ID  = 1;
const ResourceId ResourceLoadingService::FALLBACK_MESH_ID    = 2;

///------------------------------------------------------------------------------------------------

ResourceLoadingService& ResourceLoadingService::GetInstance()
{
    static ResourceLoadingService instance;
    if (!instance.mInitialized) instance.Initialize();
    return instance;
}

///------------------------------------------------------------------------------------------------

ResourceLoadingService::~ResourceLoadingService()
{
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::Initialize()
{
    using namespace strutils;
    
    // No make unique due to constructing the loaders with their private constructors
    // via friendship
    mResourceLoaders.push_back(std::unique_ptr<TextureLoader>(new TextureLoader));
    mResourceLoaders.push_back(std::unique_ptr<DataFileLoader>(new DataFileLoader));
    mResourceLoaders.push_back(std::unique_ptr<ShaderLoader>(new ShaderLoader));
    mResourceLoaders.push_back(std::unique_ptr<OBJMeshLoader>(new OBJMeshLoader));
    
    // Map resource extensions to loaders
    mResourceExtensionsToLoadersMap[StringId("bmp")]  = mResourceLoaders[0].get();
    mResourceExtensionsToLoadersMap[StringId("json")] = mResourceLoaders[1].get();
    mResourceExtensionsToLoadersMap[StringId("dat")]  = mResourceLoaders[1].get();
    mResourceExtensionsToLoadersMap[StringId("lua")]  = mResourceLoaders[1].get();
    mResourceExtensionsToLoadersMap[StringId("xml")]  = mResourceLoaders[1].get();
    mResourceExtensionsToLoadersMap[StringId("vs")]   = mResourceLoaders[2].get();
    mResourceExtensionsToLoadersMap[StringId("fs")]   = mResourceLoaders[2].get();
    mResourceExtensionsToLoadersMap[StringId("obj")]  = mResourceLoaders[3].get();
    
    for (auto& resourceLoader: mResourceLoaders)
    {
        resourceLoader->VInitialize();
    }
    
    mInitialized = true;
}

///------------------------------------------------------------------------------------------------

ResourceId ResourceLoadingService::GetResourceIdFromPath(const std::string& path)
{    
    return strutils::GetStringHash(AdjustResourcePath(path));
}

///------------------------------------------------------------------------------------------------

ResourceId ResourceLoadingService::LoadResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = strutils::GetStringHash(adjustedPath);
    
    if (mResourceMap.count(resourceId))
    {
        return resourceId;
    }
    else
    {
        LoadResourceInternal(adjustedPath, resourceId);
        return resourceId;
    }
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::LoadResources(const std::vector<std::string>& resourcePaths)
{
    for (const auto& path: resourcePaths)
    {
        LoadResource(path);
    }
}

///------------------------------------------------------------------------------------------------

bool ResourceLoadingService::DoesResourceExist(const std::string& resourcePath) const
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    std::fstream resourceFileCheck(resourcePath);
    return resourceFileCheck.operator bool();
}

///------------------------------------------------------------------------------------------------

bool ResourceLoadingService::HasLoadedResource(const std::string& resourcePath) const
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = strutils::GetStringHash(adjustedPath);
    
    return mResourceMap.count(resourceId) != 0;
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::UnloadResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = strutils::GetStringHash(adjustedPath);
    mResourceMap.erase(resourceId);
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::UnloadResource(const ResourceId resourceId)
{
    mResourceMap.erase(resourceId);
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::SetFallbackTexture(const std::string& fallbackTexturePath)
{
    auto adjustedPath = AdjustResourcePath(fallbackTexturePath);
    LoadResourceInternal(adjustedPath, FALLBACK_TEXTURE_ID);
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::SetFallbackShader(const std::string& fallbackShaderPath)
{
    auto adjustedPath = AdjustResourcePath(fallbackShaderPath);
    LoadResourceInternal(adjustedPath, FALLBACK_SHADER_ID);
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::SetFallbackMesh(const std::string& fallbackMeshPath)
{
    auto adjustedPath = AdjustResourcePath(fallbackMeshPath);
    LoadResourceInternal(adjustedPath, FALLBACK_MESH_ID);
}

///------------------------------------------------------------------------------------------------

IResource& ResourceLoadingService::GetResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = strutils::GetStringHash(adjustedPath);
    return GetResource(resourceId);
}

///------------------------------------------------------------------------------------------------

IResource& ResourceLoadingService::GetResource(const ResourceId resourceId)
{
    if (mResourceMap.count(resourceId))
    {
        return *mResourceMap[resourceId];
    }
    
    assert(false && "Resource could not be found");
    return *mResourceMap[resourceId];
}

///------------------------------------------------------------------------------------------------

void ResourceLoadingService::LoadResourceInternal(const std::string& resourcePath, const ResourceId resourceId)
{
    // Get resource extension
    const auto resourceFileExtension = fileutils::GetFileExtension(resourcePath);
    
    // Pick appropriate loader
    strutils::StringId fileExtension(fileutils::GetFileExtension(resourcePath));
    auto loadersIter = mResourceExtensionsToLoadersMap.find(fileExtension);
    if (loadersIter != mResourceExtensionsToLoadersMap.end())
    {
        auto& selectedLoader = mResourceExtensionsToLoadersMap.at(strutils::StringId(fileutils::GetFileExtension(resourcePath)));
        auto loadedResource = selectedLoader->VCreateAndLoadResource(RES_ROOT + resourcePath);
        mResourceMap[resourceId] = std::move(loadedResource);
        Log(LogType::INFO, "Loading asset: %s in %s", resourcePath.c_str(), std::to_string(resourceId).c_str());
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Unable to find loader for given extension", "A loader could not be found for extension: " + fileExtension.GetString());
    }
}

///------------------------------------------------------------------------------------------------

std::string ResourceLoadingService::AdjustResourcePath(const std::string& resourcePath) const
{    
    return !strutils::StringStartsWith(resourcePath, RES_ROOT) ? resourcePath : resourcePath.substr(RES_ROOT.size(), resourcePath.size() - RES_ROOT.size());
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
