///------------------------------------------------------------------------------------------------
///  LightRepository.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef LightRepository_h
#define LightRepository_h

///------------------------------------------------------------------------------------------------

#include "../../utils/MathUtils.h"
#include "../../utils/StringUtils.h"

#include <vector>

///------------------------------------------------------------------------------------------------

enum class LightType
{
    AMBIENT_LIGHT,
    POINT_LIGHT
};

///------------------------------------------------------------------------------------------------

class LightRepository final
{
public:
    friend class SceneRenderer;
    
    LightRepository() = default;
    ~LightRepository() = default;
    LightRepository(const LightRepository&) = delete;
    LightRepository(LightRepository&&) = delete;
    const LightRepository& operator = (const LightRepository&) = delete;
    LightRepository& operator = (LightRepository&&) = delete;
    
    static constexpr int INVALID_LIGHT_INDEX = -2;
    static constexpr int AMBIENT_LIGHT_INDEX = -1;
    int GetLightIndex(const strutils::StringId& lightName) const;
    void AddLight(const LightType lightType, const strutils::StringId& lightName, const glm::vec4& lightColor, const glm::vec3& lightPosition, const float lightPower);
    void RemoveLight(const strutils::StringId& lightName);
    
    strutils::StringId GetLightName(const int lightIndex) const;
    float GetLightPower(const int lightIndex) const;
    glm::vec3 GetLightPosition(const int lightIndex) const;
    glm::vec4 GetLightColor(const int lightIndex) const;
    
    void SetLightName(const int lightIndex, const strutils::StringId& lightName);
    void SetLightPower(const int lightIndex, const float lightPower);
    void SetLightPosition(const int lightIndex, const glm::vec3& lightPosition);
    void SetLightColor(const int lightIndex, const glm::vec4& lightColor);
    
private:
    // The data is layed out internally like this for efficient pass through to the shaders
    strutils::StringId mAmbientLightName;
    glm::vec4 mAmbientLightColor;
    std::vector<strutils::StringId> mPointLightNames;
    std::vector<glm::vec4> mPointLightColors;
    std::vector<glm::vec3> mPointLightPositions;
    std::vector<float> mPointLightPowers;
    
};///------------------------------------------------------------------------------------------------

#endif /* LightRepository_h */
