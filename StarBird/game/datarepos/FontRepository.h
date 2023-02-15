///------------------------------------------------------------------------------------------------
///  FontRepository.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef FontRepository_h
#define FontRepository_h

///------------------------------------------------------------------------------------------------

#include "../definitions/FontDefinition.h"
#include "../dataloaders/FontLoader.h"
#include "../../utils/StringUtils.h"

#include <optional>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class FontRepository final
{
public:
    static FontRepository& GetInstance();
    
    ~FontRepository() = default;
    FontRepository(const FontRepository&) = delete;
    FontRepository(FontRepository&&) = delete;
    const FontRepository& operator = (const FontRepository&) = delete;
    FontRepository& operator = (FontRepository&&) = delete;
    
    std::optional<std::reference_wrapper<const FontDefinition>> GetFont(const strutils::StringId& fontName) const;
    void LoadFont(const strutils::StringId& fontName);
    
private:
    FontRepository() = default;
    
private:
    FontLoader mLoader;
    std::unordered_map<strutils::StringId, FontDefinition, strutils::StringIdHasher> mFontMap;
};

///------------------------------------------------------------------------------------------------

#endif /* FontRepository_h */
