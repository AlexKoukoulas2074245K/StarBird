///------------------------------------------------------------------------------------------------
///  Animations.h
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/02/2023
///------------------------------------------------------------------------------------------------

#ifndef Animations_h
#define Animations_h

///------------------------------------------------------------------------------------------------

#include "../resloading/ResourceLoadingService.h"

#include <memory>

//enum class AnimationType
//{
//    SINGLE_FRAME, MULTI_FRAME, VARIABLE_TEXTURED, PULSING, SHINE, DISSOLVE
//};
//
/////------------------------------------------------------------------------------------------------
//
//
//
//struct Animation
//{
//    std::vector<resources::ResourceId> mVariableTextureResourceIds;
//    resources::ResourceId mTextureResourceId;
//    float mDuration = 0.0f;
//    float mScale = 1.0f;
//    float mAnimDtAccum = 0.0f;
//    float mAnimDtAccumSpeed = 0.0f;
//    float mPulsingEnlargementFactor = 0.0f;
//    int mTextureSheetRow = 0;
//    AnimationType mAnimationType = AnimationType::SINGLE_FRAME;
//};

///------------------------------------------------------------------------------------------------

class SceneObject;
class IAnimation
{
public:
    virtual ~IAnimation() = default;
    virtual std::unique_ptr<IAnimation> VClone() const = 0;
    virtual void VUpdate(const float dtMillis, SceneObject& sceneObject) = 0;
    virtual resources::ResourceId VGetCurrentTextureResourceId() const = 0;
    virtual float VGetDuration() const = 0;
};
    
///------------------------------------------------------------------------------------------------

class SingleFrameAnimation final: public IAnimation
{
public:
    SingleFrameAnimation(const resources::ResourceId textureResourceId);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
};

///------------------------------------------------------------------------------------------------

class MultiFrameAnimation final: public IAnimation
{
public:
    MultiFrameAnimation(const resources::ResourceId textureResourceId, const float duration, const float scale, const int textureSheetRow);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    float mDuration;
    float mScale;
    float mAnimationTime;
    int mAnimationIndex;
    int mTextureSheetRow;
};

///------------------------------------------------------------------------------------------------

class VariableTexturedAnimation final: public IAnimation
{
public:
    VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    std::vector<resources::ResourceId> mPotentialTextureResourceIds;
    resources::ResourceId mTextureResourceId;
};

///------------------------------------------------------------------------------------------------

class PulsingAnimation final: public IAnimation
{
public:
    PulsingAnimation(const resources::ResourceId textureResourceId, const float pulsingSpeed, const float pulsingEnlargementFactor);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    float mPulsingSpeed;
    float mPulsingEnlargementFactor;
    float mPulsingDtAccum;
};

///------------------------------------------------------------------------------------------------

class ShineAnimation final: public IAnimation
{
public:
    ShineAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const float shineSpeed);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mShineTextureResourceId;
    resources::ResourceId mShineShaderResourceId;
    float mShineSpeed;
    float mShineXOffset;
};

///------------------------------------------------------------------------------------------------

class DissolveAnimation final: public IAnimation
{
public:
    DissolveAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const float dissolveSpeed);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mDissolveTextureResourceId;
    resources::ResourceId mDissolveShaderResourceId;
    float mDissolveSpeed;
    float mDissolveYOffset;
};

///------------------------------------------------------------------------------------------------

#endif /* Animations_h */
