///------------------------------------------------------------------------------------------------
///  InputContext.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef InputContext_h
#define InputContext_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"

#include <string>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

struct InputContext
{
    std::string mText;
    glm::vec2 mTouchPos;
    Uint32 mEventType;
    SDL_Scancode mKeyCode;
    float mPinchDistance;
    bool mMultiGestureActive;
};

///------------------------------------------------------------------------------------------------

#endif /* InputContext_h */
