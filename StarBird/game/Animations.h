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
#include "../utils/MathUtils.h"

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
    virtual resources::ResourceId VGetCurrentEffectTextureResourceId() const = 0;
    virtual resources::ResourceId VGetCurrentMeshResourceId() const = 0;
    virtual resources::ResourceId VGetCurrentShaderResourceId() const = 0;
    virtual const glm::vec3& VGetScale() const = 0;
    virtual float VGetDuration() const = 0;
    virtual bool VGetBodyRenderingEnabled() const = 0;
};

///------------------------------------------------------------------------------------------------

class SingleFrameAnimation final: public IAnimation
{
public:
    SingleFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class MultiFrameAnimation final: public IAnimation
{
public:
    MultiFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float duration, const int textureSheetRow, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    float mDuration;
    float mAnimationTime;
    int mAnimationIndex;
    int mTextureSheetRow;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class VariableTexturedAnimation final: public IAnimation
{
public:
    VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    std::vector<resources::ResourceId> mPotentialTextureResourceIds;
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class PulsingAnimation final: public IAnimation
{
public:
    PulsingAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float delayedStartMillis, const float pulsingSpeed, const float pulsingEnlargementFactor, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    float mDelayedStartMillis;
    float mPulsingSpeed;
    float mPulsingEnlargementFactor;
    float mPulsingDtAccum;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class BezierCurvePathAnimation final: public IAnimation
{
public:
    BezierCurvePathAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const math::BezierCurve& pathCurve, const float curveTraversalSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    math::BezierCurve mPathCurve;
    float mCurveTraversalSpeed;
    float mCurveTraversalProgress;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class ShineAnimation final: public IAnimation
{
public:
    ShineAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float shineSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mShineTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    float mShineSpeed;
    float mShineXOffset;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class DissolveAnimation final: public IAnimation
{
public:
    DissolveAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float dissolveSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mDissolveTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    float mDissolveSpeed;
    float mDissolveYOffset;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class NebulaAnimation final: public IAnimation
{
public:
    NebulaAnimation(SceneObject* sceneObject, const resources::ResourceId noiseTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float noiseMovementSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    resources::ResourceId mNoiseTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    glm::vec2 mNoiseMovementDirection;
    float mNoiseMovementSpeed;
    bool mBodyRenderingEnabled;
};

///------------------------------------------------------------------------------------------------

class RotationAnimation final: public IAnimation
{
public:
    enum class RotationAxis
    {
        X, Y, Z
    };
    
    enum class RotationMode
    {
        ROTATE_CONTINUALLY, ROTATE_TO_TARGET_ONCE, ROTATE_TO_TARGET_AND_BACK_ONCE, ROTATE_TO_TARGET_AND_BACK_CONTINUALLY
    };
    
    RotationAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const RotationMode rotationMode, const RotationAxis rotationAxis, const float rotationDegrees, const float rotationSpeed, const bool bodyRenderingEnabled);
    
    void SetRotationMode(const RotationMode rotationMode);
    
    std::unique_ptr<IAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentTextureResourceId() const override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    resources::ResourceId VGetCurrentMeshResourceId() const override;
    resources::ResourceId VGetCurrentShaderResourceId() const override;
    const glm::vec3& VGetScale() const override;
    float VGetDuration() const override;
    bool VGetBodyRenderingEnabled() const override;
    
private:
    void OnSingleRotationFinished();
    
private:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    RotationMode mRotationMode;
    RotationAxis mRotationAxis;
    float mRotationRadians;
    float mPreviousRotationRadians;
    float mRotationSpeed;
    float mRotationDtAccum;
    bool mLeftHandRotation;
    bool mBodyRenderingEnabled;
    bool mFinishedRotationOnce;
};

///------------------------------------------------------------------------------------------------

#endif /* Animations_h */
