///------------------------------------------------------------------------------------------------
///  LightRepository.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "LightRepository.h"

///------------------------------------------------------------------------------------------------

int LightRepository::GetLightIndex(const strutils::StringId& lightName) const
{
    if (mAmbientLightName == lightName)
    {
        return AMBIENT_LIGHT_INDEX;
    }
    
    auto findIter = std::find(mPointLightNames.begin(), mPointLightNames.end(), lightName);
    if (findIter == mPointLightNames.end())
    {
        return INVALID_LIGHT_INDEX;
    }
    return static_cast<int>(findIter - mPointLightNames.begin());
}

///------------------------------------------------------------------------------------------------

void LightRepository::AddLight(const LightType lightType, const strutils::StringId& lightName, const glm::vec4& lightColor, const glm::vec3& lightPosition, const float lightPower)
{
    if (lightType == LightType::AMBIENT_LIGHT)
    {
        mAmbientLightName = lightName;
        mAmbientLightColor = lightColor;
    }
    else
    {
        mPointLightNames.push_back(lightName);
        mPointLightColors.push_back(lightColor);
        mPointLightPositions.push_back(lightPosition);
        mPointLightPowers.push_back(lightPower);
    }
}

///------------------------------------------------------------------------------------------------

void LightRepository::RemoveLight(const strutils::StringId& lightName)
{
    auto lightIndex = GetLightIndex(lightName);
    if (lightIndex != INVALID_LIGHT_INDEX)
    {
        if (lightIndex == AMBIENT_LIGHT_INDEX)
        {
            mAmbientLightName = strutils::StringId();
            mAmbientLightColor = glm::vec4(0.0f);
        }
        else
        {
            mPointLightNames.erase(mPointLightNames.begin() + lightIndex);
            mPointLightColors.erase(mPointLightColors.begin() + lightIndex);
            mPointLightPositions.erase(mPointLightPositions.begin() + lightIndex);
            mPointLightPowers.erase(mPointLightPowers.begin() + lightIndex);
        }
    }
}

///------------------------------------------------------------------------------------------------

strutils::StringId LightRepository::GetLightName(const int lightIndex) const
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return strutils::StringId();
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        return mAmbientLightName;
    }
    
    return mPointLightNames[lightIndex];
}

///------------------------------------------------------------------------------------------------

float LightRepository::GetLightPower(const int lightIndex) const
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return 0.0f;
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        return 0.0f;
    }
    
    return mPointLightPowers[lightIndex];
}

///------------------------------------------------------------------------------------------------
glm::vec3 LightRepository::GetLightPosition(const int lightIndex) const
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return glm::vec3(0.0f);
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        return glm::vec3(0.0f);
    }
    
    return mPointLightPositions[lightIndex];
}

///------------------------------------------------------------------------------------------------

glm::vec4 LightRepository::GetLightColor(const int lightIndex) const
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return glm::vec4(0.0f);
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        return mAmbientLightColor;
    }
    
    return mPointLightColors[lightIndex];
}

///------------------------------------------------------------------------------------------------

void LightRepository::SetLightName(const int lightIndex, const strutils::StringId& lightName)
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return;
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        mAmbientLightName = lightName;
    }
    else
    {
        mPointLightNames[lightIndex] = lightName;
    }
}

///------------------------------------------------------------------------------------------------

void LightRepository::SetLightPower(const int lightIndex, const float lightPower)
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return;
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        return;
    }
    else
    {
        mPointLightPowers[lightIndex] = lightPower;
    }
}

///------------------------------------------------------------------------------------------------

void LightRepository::SetLightPosition(const int lightIndex, const glm::vec3& lightPosition)
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return;
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        return;
    }
    else
    {
        mPointLightPositions[lightIndex] = lightPosition;
    }
}

///------------------------------------------------------------------------------------------------

void LightRepository::SetLightColor(const int lightIndex, const glm::vec4& lightColor)
{
    if (lightIndex == INVALID_LIGHT_INDEX)
    {
        return;
    }
    
    if (lightIndex == AMBIENT_LIGHT_INDEX)
    {
        mAmbientLightColor = lightColor;
    }
    else
    {
        mPointLightColors[lightIndex] = lightColor;
    }
}

///------------------------------------------------------------------------------------------------
