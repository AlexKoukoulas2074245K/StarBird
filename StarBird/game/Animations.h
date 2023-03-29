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
class BaseAnimation
{
public:
    BaseAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled);
    
    virtual ~BaseAnimation() = default;
    virtual std::unique_ptr<BaseAnimation> VClone() const = 0;
    virtual void VUpdate(const float dtMillis, SceneObject& sceneObject);
    virtual bool VIsPaused() const;
    virtual void VPause();
    virtual void VResume();
    virtual resources::ResourceId VGetCurrentTextureResourceId() const;
    virtual resources::ResourceId VGetCurrentEffectTextureResourceId() const;
    virtual resources::ResourceId VGetCurrentMeshResourceId() const;
    virtual resources::ResourceId VGetCurrentShaderResourceId() const;
    virtual const glm::vec3& VGetScale() const;
    virtual float VGetDuration() const;
    virtual bool VGetBodyRenderingEnabled() const;
    
protected:
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mShaderResourceId;
    glm::vec3 mScale;
    bool mBodyRenderingEnabled;
    bool mPaused;
};

///------------------------------------------------------------------------------------------------

class SingleFrameAnimation final: public BaseAnimation
{
public:
    SingleFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
};

///------------------------------------------------------------------------------------------------

class MultiFrameAnimation final: public BaseAnimation
{
public:
    MultiFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float duration, const int textureSheetRow, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    float VGetDuration() const override;
    
private:
    float mDuration;
    float mAnimationTime;
    int mAnimationIndex;
    int mTextureSheetRow;
};

///------------------------------------------------------------------------------------------------

class VariableTexturedAnimation final: public BaseAnimation
{
public:
    VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    
private:
    std::vector<resources::ResourceId> mPotentialTextureResourceIds;
};

///------------------------------------------------------------------------------------------------

class PulsingAnimation final: public BaseAnimation
{
public:
    PulsingAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float delayedStartMillis, const float pulsingSpeed, const float pulsingEnlargementFactor, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;

    float VGetDuration() const override;
    
private:
    float mDelayedStartMillis;
    float mPulsingSpeed;
    float mPulsingEnlargementFactor;
    float mPulsingDtAccum;
};

///------------------------------------------------------------------------------------------------

class BezierCurvePathAnimation final: public BaseAnimation
{
public:
    BezierCurvePathAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const math::BezierCurve& pathCurve, const float curveTraversalSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    float VGetDuration() const override;
    
private:
    math::BezierCurve mPathCurve;
    float mCurveTraversalSpeed;
    float mCurveTraversalProgress;
};

///------------------------------------------------------------------------------------------------

class ShineAnimation final: public BaseAnimation
{
public:
    ShineAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float shineSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mShineTextureResourceId;
    float mShineSpeed;
    float mShineXOffset;
};

///------------------------------------------------------------------------------------------------

class DissolveAnimation final: public BaseAnimation
{
public:
    DissolveAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float dissolveSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    resources::ResourceId VGetCurrentEffectTextureResourceId() const override;
    float VGetDuration() const override;
    
private:
    resources::ResourceId mDissolveTextureResourceId;
    float mDissolveSpeed;
    float mDissolveYOffset;
};

///------------------------------------------------------------------------------------------------

class NebulaAnimation final: public BaseAnimation
{
public:
    NebulaAnimation(SceneObject* sceneObject, const resources::ResourceId noiseTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float noiseMovementSpeed, const bool bodyRenderingEnabled);
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    
private:
    glm::vec2 mNoiseMovementDirection;
    float mNoiseMovementSpeed;
};

///------------------------------------------------------------------------------------------------

class RotationAnimation final: public BaseAnimation
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
    
    std::unique_ptr<BaseAnimation> VClone() const override;
    void VUpdate(const float dtMillis, SceneObject& sceneObject) override;
    float VGetDuration() const override;
    
private:
    void OnSingleRotationFinished();
    
private:
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
