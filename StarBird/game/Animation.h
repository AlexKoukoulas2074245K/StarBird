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

///------------------------------------------------------------------------------------------------

struct Animation
{
    resources::ResourceId mTextureResourceId;
    float mDuration = 0.0f;
    float mScale = 1.0f;
    int mTextureSheetRow = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* Animation_h */