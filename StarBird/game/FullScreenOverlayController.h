///------------------------------------------------------------------------------------------------
///  FullScreenOverlayController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef FullScreenOverlayController_h
#define FullScreenOverlayController_h

///------------------------------------------------------------------------------------------------

#include <functional>

///------------------------------------------------------------------------------------------------

class Scene;
class FullScreenOverlayController final
{
public:
    using CallbackType = std::function<void()>;
    
    FullScreenOverlayController(Scene& scene, const float darkeningSpeed, const float maxDarkeningValue, const bool pauseAtMidPoint, CallbackType midwayCallback = nullptr, CallbackType completionCallback = nullptr);
    
    void Update(const float dtMillis);
    void Resume();
    bool IsFinished() const;
    
private:
    Scene& mScene;
    const float mDarkeningSpeed;
    const float mMaxDarkeningValue;
    const bool mPauseAtMidPoint;
    float mDarkeningValue;
    CallbackType mMidwayCallback;
    CallbackType mCompletionCallback;
    bool mDarkening;
    bool mFinished;
    bool mPaused;
};

///------------------------------------------------------------------------------------------------

#endif /* FullScreenOverlayController_h */
