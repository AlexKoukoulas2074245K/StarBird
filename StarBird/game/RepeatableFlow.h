///------------------------------------------------------------------------------------------------
///  RepeatableFlow.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef RepeatableFlow_h
#define RepeatableFlow_h

///------------------------------------------------------------------------------------------------

class RepeatableFlow
{
public:
    enum class RepeatPolicy
    {
        ONCE, REPEAT
    };
    
    using CallbackT = std::function<void()>;
    
    RepeatableFlow(CallbackT callback, float afterMs, RepeatPolicy repeatPolicy):
          mCallback(callback)
        , mTargetDuration(afterMs)
        , mTicksLeft(afterMs)
        , mRepeatPolicy(repeatPolicy)
        , mIsRunning(true)
    {}
    
    bool IsRunning() const { return mIsRunning; }
    
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
};

///------------------------------------------------------------------------------------------------

#endif /* RepeatableFlow_h */
