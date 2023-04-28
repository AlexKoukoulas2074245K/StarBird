///------------------------------------------------------------------------------------------------
///  GUISceneLoader.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/02/2023
///------------------------------------------------------------------------------------------------

#include "GUISceneLoader.h"
#include "../../utils/Logging.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

GUISceneLoader::GUISceneLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("GUIElement"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        GUIElementDefinition guiElement;
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            guiElement.mSceneObjectName = strutils::StringId(name->value());
        }
        
        auto* fontName = node->first_attribute("fontName");
        if (fontName)
        {
            guiElement.mFontName = strutils::StringId(fontName->value());
        }
        
        auto* position = node->first_attribute("position");
        if (position)
        {
            auto positionComponents = strutils::StringSplit(std::string(position->value()), ',');
            guiElement.mPosition.x = std::stof(positionComponents[0]);
            guiElement.mPosition.y = std::stof(positionComponents[1]);
            guiElement.mPosition.z = std::stof(positionComponents[2]);
        }
        
        auto* scale = node->first_attribute("scale");
        if (scale)
        {
            auto scaleComponents = strutils::StringSplit(std::string(scale->value()), ',');
            guiElement.mScale.x = std::stof(scaleComponents[0]);
            guiElement.mScale.y = std::stof(scaleComponents[1]);
            guiElement.mScale.z = 1.0f;
        }
        
        auto* texture = node->first_attribute("texture");
        if (texture)
        {
            guiElement.mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(texture->value()) + ".bmp");
        }
        
        auto* shader = node->first_attribute("shader");
        if (shader)
        {
            guiElement.mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + std::string(shader->value()) + ".vs");
        }
        
        auto* text = node->first_attribute("text");
        if (text)
        {
            guiElement.mText = std::string(text->value());
        }
        
        auto* invisible = node->first_attribute("invisible");
        if (invisible)
        {
            guiElement.mInvisible = strcmp(invisible->value(), "true") == 0;
        }
        
        mConstructedScene.mGUIElements.push_back(guiElement);
    });
}

///------------------------------------------------------------------------------------------------

GUISceneDefinition& GUISceneLoader::LoadGUIScene(const std::string& sceneName)
{
    mConstructedScene = GUISceneDefinition();
    mConstructedScene.mSceneName = strutils::StringId(sceneName);
    
    BaseGameDataLoader::LoadData(sceneName);
    
    return mConstructedScene;
}

///------------------------------------------------------------------------------------------------
