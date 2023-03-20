///------------------------------------------------------------------------------------------------
///  IUpdater.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef IUpdater_h
#define IUpdater_h

///------------------------------------------------------------------------------------------------

#include "SceneObject.h"

#include <SDL_events.h>
#include <string>
#include <vector>

///------------------------------------------------------------------------------------------------

class IUpdater
{
public:
    virtual ~IUpdater() = default;
    
    virtual void Update(std::vector<SceneObject>& sceneObjects, const float dtMillis) = 0;
    virtual void OnAppStateChange(Uint32 event) = 0;
    virtual std::string GetDescription() const = 0;
    
#ifdef DEBUG
    virtual void OpenDebugConsole() = 0;
#endif
};

///------------------------------------------------------------------------------------------------

#endif /* IUpdater_h */
