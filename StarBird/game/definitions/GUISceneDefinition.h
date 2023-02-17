///------------------------------------------------------------------------------------------------
///  GUISceneDefinition.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/02/2023
///------------------------------------------------------------------------------------------------

#ifndef GUISceneDefinition_h
#define GUISceneDefinition_h

///------------------------------------------------------------------------------------------------

#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/MathUtils.h"
#include "../../utils/StringUtils.h"

#include <vector>

///------------------------------------------------------------------------------------------------

struct GUIElementDefinition
{
    strutils::StringId mFontName;
    strutils::StringId mSceneObjectName;
    glm::vec3 mPosition;
    glm::vec3 mScale;
    resources::ResourceId mShaderResourceId;
    resources::ResourceId mTextureResourceId;
    std::string mText;
};

///------------------------------------------------------------------------------------------------

struct GUISceneDefinition
{
    strutils::StringId mSceneName = strutils::StringId();
    std::vector<GUIElementDefinition> mGUIElements;
};

///------------------------------------------------------------------------------------------------

#endif /* GUISceneDefinition_h */
