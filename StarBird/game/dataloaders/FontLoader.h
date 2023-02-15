///------------------------------------------------------------------------------------------------
///  FontLoader.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/02/2023
///------------------------------------------------------------------------------------------------

#ifndef FontLoader_h
#define FontLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "../definitions/FontDefinition.h"

#include <string>

///------------------------------------------------------------------------------------------------

class FontLoader: public BaseGameDataLoader
{
public:
    FontLoader();
    FontDefinition& LoadFont(const std::string& fontName);

private:
    FontDefinition mConstructedFont;
};

///------------------------------------------------------------------------------------------------

#endif /* FontLoader_h */
