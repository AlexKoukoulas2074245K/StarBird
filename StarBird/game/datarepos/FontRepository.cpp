///------------------------------------------------------------------------------------------------
///  FontRepository.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/02/2023
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "../../utils/OSMessageBox.h"

///------------------------------------------------------------------------------------------------

FontRepository& FontRepository::GetInstance()
{
    static FontRepository instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<const FontDefinition>> FontRepository::GetFont(const strutils::StringId& fontName) const
{
    auto findIter = mFontMap.find(fontName);
    if (findIter != mFontMap.end())
    {
        return std::optional<std::reference_wrapper<const FontDefinition>>{findIter->second};
    }
    
    ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Cannot find font", fontName.GetString().c_str());
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void FontRepository::LoadFont(const strutils::StringId& fontName)
{
    mFontMap[fontName] = mLoader.LoadFont(fontName.GetString());
}

///------------------------------------------------------------------------------------------------
