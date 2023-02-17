///------------------------------------------------------------------------------------------------
///  GUISceneLoader.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/02/2023
///------------------------------------------------------------------------------------------------

#ifndef GUISceneLoader_h
#define GUISceneLoader_h

///------------------------------------------------------------------------------------------------

#include "BaseGameDataLoader.h"
#include "../definitions/GUISceneDefinition.h"

#include <string>

///------------------------------------------------------------------------------------------------

class GUISceneLoader: public BaseGameDataLoader
{
public:
    GUISceneLoader();
    GUISceneDefinition& LoadGUIScene(const std::string& sceneName);

private:
    GUISceneDefinition mConstructedScene;
};

///------------------------------------------------------------------------------------------------

#endif /* GUISceneDefinition_h */
