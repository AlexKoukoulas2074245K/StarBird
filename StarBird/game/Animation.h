///------------------------------------------------------------------------------------------------
///  Animation.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Animation_h
#define Animation_h

///------------------------------------------------------------------------------------------------

#include "../resloading/ResourceLoadingService.h"

#include <vector>

///------------------------------------------------------------------------------------------------

enum class AnimationType
{
    SINGLE_FRAME, MULTI_FRAME, VARIABLE_TEXTURED, PULSING
};

///------------------------------------------------------------------------------------------------

struct Animation
{
    std::vector<resources::ResourceId> mVariableTextureResourceIds;
    resources::ResourceId mTextureResourceId;
    float mDuration = 0.0f;
    float mScale = 1.0f;
    float mAnimDtAccum = 0.0f;
    float mAnimDtAccumSpeed = 0.0f;
    float mPulsingEnlargementFactor = 0.0f;
    int mTextureSheetRow = 0;
    AnimationType mAnimationType = AnimationType::SINGLE_FRAME;
};

///------------------------------------------------------------------------------------------------

#endif /* Animation_h */
