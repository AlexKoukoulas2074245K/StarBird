///------------------------------------------------------------------------------------------------
///  FullScreenOverlayController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef FullScreenOverlayController_h
#define FullScreenOverlayController_h

///------------------------------------------------------------------------------------------------

#include "GameConstants.h"
#include "../utils/StringUtils.h"

#include <functional>

///------------------------------------------------------------------------------------------------

class Scene;
class FullScreenOverlayController final
{
public:
    using CallbackType = std::function<void()>;
    
    FullScreenOverlayController(Scene& scene, const float darkeningSpeed, const float maxDarkeningValue, const bool pauseAtMidPoint, CallbackType midwayCallback = nullptr, CallbackType completionCallback = nullptr, const float customZ = 3.5f, const strutils::StringId sceneObjectName = game_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME, const bool crossSceneLifetime = true);
    
    void Update(const float dtMillis);
    void Resume();
    bool IsFinished() const;
    
private:
    Scene& mScene;
    const strutils::StringId mSceneObjectName;
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
