///------------------------------------------------------------------------------------------------
///  RepeatableFlow.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef RepeatableFlow_h
#define RepeatableFlow_h

///------------------------------------------------------------------------------------------------

#include "../utils/StringUtils.h"

///------------------------------------------------------------------------------------------------

class RepeatableFlow
{
public:
    enum class RepeatPolicy
    {
        ONCE, REPEAT
    };
    
    using CallbackT = std::function<void()>;
    
    RepeatableFlow(CallbackT callback, float durationMillis, RepeatPolicy repeatPolicy, strutils::StringId name = strutils::StringId()):
          mCallback(callback)
        , mTargetDuration(durationMillis)
        , mTicksLeft(durationMillis)
        , mRepeatPolicy(repeatPolicy)
        , mIsRunning(true)
        , mName(name)
    {}
    
    inline bool IsRunning() const { return mIsRunning; }
    inline const strutils::StringId& GetName() const { return mName; }
    inline float GetDuration() const { return mTargetDuration; }
    inline float GetTicksLeft() const { return mTicksLeft; }
    
    inline void ForceFinish() { mIsRunning = false; }
    inline void SetDuration(float durationMillis) { mTargetDuration = durationMillis; }
    
    void Update(float dt)
    {
        if (!mIsRunning) return;
        
        mTicksLeft -= dt;
        if (mTicksLeft <= 0.0f)
        {
            mCallback();
            if (mRepeatPolicy == RepeatPolicy::REPEAT)
            {
                mTicksLeft = mTargetDuration;
            }
            else
            {
                mIsRunning = false;
            }
        }
    }
    
private:
    CallbackT mCallback;
    float mTargetDuration;
    float mTicksLeft;
    RepeatPolicy mRepeatPolicy;
    bool mIsRunning;
    strutils::StringId mName;
};

///------------------------------------------------------------------------------------------------

#endif /* RepeatableFlow_h */
