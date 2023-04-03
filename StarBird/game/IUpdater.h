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
#include "states/BaseGameState.h"

#include <SDL_events.h>
#include <string>
#include <vector>

///------------------------------------------------------------------------------------------------

class IUpdater
{
public:
    virtual ~IUpdater() = default;
    
    virtual PostStateUpdateDirective VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis) = 0;
    virtual void VOnAppStateChange(Uint32 event) = 0;
    virtual std::string VGetDescription() const = 0;
    
#ifdef DEBUG
    virtual void VOpenDebugConsole() = 0;
#endif
};

///------------------------------------------------------------------------------------------------

#endif /* IUpdater_h */
