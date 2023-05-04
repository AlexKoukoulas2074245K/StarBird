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
#include <SDL.h>

///------------------------------------------------------------------------------------------------

struct InputContext
{
    std::string mText;
    glm::vec2 mTouchPos;
    glm::vec2 mRawAccelerometerValues;
    Uint32 mEventType;
    SDL_Scancode mKeyCode;
    SDL_Joystick* mJoystick;
    float mPinchDistance;
    bool mMultiGestureActive;
};

///------------------------------------------------------------------------------------------------

#endif /* InputContext_h */
