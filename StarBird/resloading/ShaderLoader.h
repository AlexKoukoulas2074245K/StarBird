///------------------------------------------------------------------------------------------------
///  ShaderLoader.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef ShaderLoader_h
#define ShaderLoader_h

///------------------------------------------------------------------------------------------------

#include "IResourceLoader.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

using GLuint = unsigned int;

///------------------------------------------------------------------------------------------------

class ShaderLoader final : public IResourceLoader
{
    friend class ResourceLoadingService;

public:
    void VInitialize() override;
    std::unique_ptr<IResource> VCreateAndLoadResource(const std::string& path) const override;

private:
    static const std::string VERTEX_SHADER_FILE_EXTENSION;
    static const std::string FRAGMENT_SHADER_FILE_EXTENSION;
    
    ShaderLoader() = default;
    
    std::string ReadFileContents(const std::string& filePath) const;
    void ReplaceIncludeDirectives(std::string& shaderSource) const;
    std::unordered_map<strutils::StringId, GLuint, strutils::StringIdHasher> GetUniformNamesToLocationsMap
    (
        const GLuint programId,
        const std::string& shaderName, 
        const std::string& vertexShaderFileContents,
        const std::string& fragmentShaderFileContents,
        std::vector<strutils::StringId>& samplerNamesInOrder
    ) const;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* ShaderLoader_h */
