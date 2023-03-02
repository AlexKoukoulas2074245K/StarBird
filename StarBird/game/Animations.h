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

///------------------------------------------------------------------------------------------------

class SceneObject;
class IAnimation
{
public:
    virtual ~IAnimation() = default;
    virtual std::unique_ptr<IAnimation> VClone() const = 0;
    virtual void VUpdate(const float dtMillis, SceneObject& sceneObject) = 0;
    virtual resources::ResourceId VGetCurrentTextureResourceId() const = 0;
    virtual resources::ResourceId VGetCurrentMeshResourceId() const = 0;
    virtual resources::ResourceId VGetCurrentShaderResourceId() const = 0;
    virtual float VGetDuration() const = 0;
};
    
///------------------------------------------------------------------------------------------------

class SingleFrameAnimation final: public IAnimation
{
public:
    SingleFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
};

///------------------------------------------------------------------------------------------------

class MultiFrameAnimation final: public IAnimation
{
public:
    MultiFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float duration, const float scale, const int textureSheetRow);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
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
    VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    std::vector<resources::ResourceId> mPotentialTextureResourceIds;
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
};

///------------------------------------------------------------------------------------------------

class PulsingAnimation final: public IAnimation
{
public:
    PulsingAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float pulsingSpeed, const float pulsingEnlargementFactor);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    float mPulsingSpeed;
    float mPulsingEnlargementFactor;
    float mPulsingDtAccum;
};

///------------------------------------------------------------------------------------------------

class ShineAnimation final: public IAnimation
{
public:
    ShineAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float shineSpeed);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mShineTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    float mShineSpeed;
    float mShineXOffset;
};

///------------------------------------------------------------------------------------------------

class DissolveAnimation final: public IAnimation
{
public:
    DissolveAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float dissolveSpeed);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mDissolveTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    
    float mDissolveSpeed;
    float mDissolveYOffset;
};

///------------------------------------------------------------------------------------------------

class RotationAnimation final: public IAnimation
{
public:
    enum class RotationAxis
    {
        X, Y, Z
    };
    
    RotationAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const RotationAxis rotationAxis, const float rotationDegrees, const float rotationSpeed);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    RotationAxis mRotationAxis;
    float mRotationRadians;
    float mRotationSpeed;
    float mRotationDtAccum;
    bool mLeftHandRotation;
};

///------------------------------------------------------------------------------------------------

#endif /* Animations_h */
