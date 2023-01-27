///------------------------------------------------------------------------------------------------
///  ShaderResource.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef ShaderResource_h
#define ShaderResource_h

///------------------------------------------------------------------------------------------------

#include "IResource.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include <string>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

using GLuint = unsigned int;

///------------------------------------------------------------------------------------------------

class ShaderResource final: public IResource
{
public:
    ShaderResource() = default;
    ShaderResource
    (
        const std::unordered_map<strutils::StringId, GLuint, strutils::StringIdHasher>& uniformNamesToLocations,
        const GLuint programId
    );
    ShaderResource& operator = (const ShaderResource&);
    ShaderResource(const ShaderResource&);
    
public:
    bool SetMatrix4fv(const strutils::StringId& uniformName, const glm::mat4& matrix, const GLuint count = 1, const bool transpose = false) const;
    bool SetMatrix4Array(const strutils::StringId& uniformName, const std::vector<glm::mat4>& values) const;
    bool SetFloatVec4Array(const strutils::StringId& uniformName, const std::vector<glm::vec4>& values) const;
    bool SetFloatVec3Array(const strutils::StringId& uniformName, const std::vector<glm::vec3>& values) const;
    bool SetFloatVec4(const strutils::StringId& uniformName, const glm::vec4& vec) const;
    bool SetFloatVec3(const strutils::StringId& uniformName, const glm::vec3& vec) const;
    bool SetFloat(const strutils::StringId& uniformName, const float value) const;
    bool SetFloatArray(const strutils::StringId& uniformName, const std::vector<float>& values) const;
    bool SetInt(const strutils::StringId& uniformName, const int value) const;
    bool SetBool(const strutils::StringId& uniformName, const bool value) const;

    GLuint GetProgramId() const;    

    const std::unordered_map<strutils::StringId, GLuint, strutils::StringIdHasher>& GetUniformNamesToLocations() const;

    void CopyConstruction(const ShaderResource&);
    
private:
    std::unordered_map<strutils::StringId, GLuint, strutils::StringIdHasher> mShaderUniformNamesToLocations;
    GLuint mProgramId;    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* ShaderResource_h */
