///------------------------------------------------------------------------------------------------
///  Animations.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/02/2023
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "SceneObject.h"
#include "../utils/Logging.h"
#include "../utils/MathUtils.h"

#include "../resloading/TextureResource.h"

///------------------------------------------------------------------------------------------------

SingleFrameAnimation::SingleFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId)
    : mTextureResourceId(textureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
{
}

std::unique_ptr<IAnimation> SingleFrameAnimation::VClone() const
{
    return std::make_unique<SingleFrameAnimation>(*this);
}

void SingleFrameAnimation::VUpdate(const float, SceneObject&)
{
}

resources::ResourceId SingleFrameAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId SingleFrameAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId SingleFrameAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float SingleFrameAnimation::VGetDuration() const
{
    return 0.0f;
}

///------------------------------------------------------------------------------------------------

MultiFrameAnimation::MultiFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float duration, const float scale, const int textureSheetRow)
    : mTextureResourceId(textureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    , mDuration(duration)
    , mScale(scale)
    , mAnimationTime(0.0f)
    , mTextureSheetRow(textureSheetRow)
    , mAnimationIndex(0)
{
}

std::unique_ptr<IAnimation> MultiFrameAnimation::VClone() const
{
    return std::make_unique<MultiFrameAnimation>(*this);
}

void MultiFrameAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    const auto& currentTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(mTextureResourceId);
    const auto& sheetMetaDataCurrentRow = currentTexture.GetSheetMetadata()->mRowMetadata[mTextureSheetRow];
    const auto animFrameTime = mDuration/sheetMetaDataCurrentRow.mColMetadata.size();
    
    if (mDuration > 0.0f)
    {
        mAnimationTime += dtMillis;
        if (mAnimationTime >= animFrameTime)
        {
            mAnimationTime = 0.0f;
            mAnimationIndex = (mAnimationIndex + 1) % sheetMetaDataCurrentRow.mColMetadata.size();
        }
    }
    
    sceneObject.mShaderBoolUniformValues[scene_object_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::MIN_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).minU;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::MIN_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).minV;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::MAX_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).maxU;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::MAX_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).maxV;
    sceneObject.mCustomScale = glm::vec3(mScale, mScale, 1.0f);
}

resources::ResourceId MultiFrameAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId MultiFrameAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId MultiFrameAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float MultiFrameAnimation::VGetDuration() const
{
    return mDuration;
}

///------------------------------------------------------------------------------------------------

VariableTexturedAnimation::VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId)
    : mPotentialTextureResourceIds(potentialTextureResourceIds)
    , mTextureResourceId(potentialTextureResourceIds.at(math::RandomInt(0, static_cast<int>(potentialTextureResourceIds.size()) - 1)))
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    
{
}
           
std::unique_ptr<IAnimation> VariableTexturedAnimation::VClone() const
{
    return std::make_unique<VariableTexturedAnimation>(mPotentialTextureResourceIds, mMeshResourceId, mShaderResourceId);
}

void VariableTexturedAnimation::VUpdate(const float, SceneObject& sceneObject)
{
}

resources::ResourceId VariableTexturedAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId VariableTexturedAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId VariableTexturedAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float VariableTexturedAnimation::VGetDuration() const
{
    return 0.0f;
}

///------------------------------------------------------------------------------------------------

PulsingAnimation::PulsingAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float pulsingSpeed, const float pulsingEnlargementFactor)
    : mTextureResourceId(textureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    , mPulsingSpeed(pulsingSpeed)
    , mPulsingEnlargementFactor(pulsingEnlargementFactor)
    , mPulsingDtAccum(0.0f)

{
}

std::unique_ptr<IAnimation> PulsingAnimation::VClone() const
{
    return std::make_unique<PulsingAnimation>(*this);
}

void PulsingAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    mPulsingDtAccum += dtMillis * mPulsingSpeed;
    sceneObject.mCustomScale += math::Sinf(mPulsingDtAccum) * mPulsingEnlargementFactor;
}

resources::ResourceId PulsingAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId PulsingAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId PulsingAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float PulsingAnimation::VGetDuration() const
{
    return mPulsingEnlargementFactor;
}

///------------------------------------------------------------------------------------------------

ShineAnimation::ShineAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float shineSpeed)
    : mTextureResourceId(textureResourceId)
    , mShineTextureResourceId(shineTextureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    , mShineSpeed(shineSpeed)
    , mShineXOffset(scene_object_constants::SHINE_EFFECT_X_OFFSET_INIT_VAL)
{
    if (sceneObject)
    {
        sceneObject->mShaderFloatUniformValues[scene_object_constants::SHINE_X_OFFSET_UNIFORM_NAME] = mShineXOffset;
    }
}

std::unique_ptr<IAnimation> ShineAnimation::VClone() const
{
    return std::make_unique<ShineAnimation>(*this);
}

void ShineAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    sceneObject.mShaderEffectTextureResourceId = mShineTextureResourceId;
    mShineXOffset -= mShineSpeed * dtMillis;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::SHINE_X_OFFSET_UNIFORM_NAME] = mShineXOffset;
}

resources::ResourceId ShineAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId ShineAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId ShineAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float ShineAnimation::VGetDuration() const
{
    return math::Abs(scene_object_constants::SHINE_EFFECT_X_OFFSET_END_VAL - scene_object_constants::SHINE_EFFECT_X_OFFSET_INIT_VAL)/mShineSpeed;
}

///------------------------------------------------------------------------------------------------

DissolveAnimation::DissolveAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const float dissolveSpeed)
    : mTextureResourceId(textureResourceId)
    , mDissolveTextureResourceId(dissolveTextureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    , mDissolveSpeed(dissolveSpeed)
    , mDissolveYOffset(scene_object_constants::DISSOLVE_EFFECT_Y_INIT_VAL)
{
    if (sceneObject)
    {
        sceneObject->mShaderFloatUniformValues[scene_object_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] = mDissolveYOffset;
    }
}

std::unique_ptr<IAnimation> DissolveAnimation::VClone() const
{
    return std::make_unique<DissolveAnimation>(*this);
}

void DissolveAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    sceneObject.mShaderEffectTextureResourceId = mDissolveTextureResourceId;
    mDissolveYOffset -= mDissolveSpeed * dtMillis;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] = mDissolveYOffset;
}

resources::ResourceId DissolveAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId DissolveAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId DissolveAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float DissolveAnimation::VGetDuration() const
{
    return scene_object_constants::DISSOLVE_EFFECT_Y_INIT_VAL/mDissolveSpeed;
}

///------------------------------------------------------------------------------------------------

RotationAnimation::RotationAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const RotationAxis rotationAxis, const float rotationDegrees, const float rotationSpeed)
    : mTextureResourceId(textureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    , mRotationAxis(rotationAxis)
    , mRotationRadians(glm::radians(rotationDegrees))
    , mRotationSpeed(rotationSpeed)
    , mRotationDtAccum(0.0f)
    , mLeftHandRotation(rotationDegrees < 0.0f)
{
}

std::unique_ptr<IAnimation> RotationAnimation::VClone() const
{
    return std::make_unique<RotationAnimation>(*this);
}

void RotationAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    if (mLeftHandRotation)
    {
        mRotationDtAccum -= dtMillis * mRotationSpeed;
        if (mRotationDtAccum < mRotationRadians)
        {
            mRotationDtAccum = mRotationRadians;
        }
    }
    else
    {
        mRotationDtAccum += dtMillis * mRotationSpeed;
        if (mRotationDtAccum > mRotationRadians)
        {
            mRotationDtAccum = mRotationRadians;
        }
    }
    
    switch (mRotationAxis)
    {
        case RotationAxis::X: sceneObject.mCustomRotation.x = mRotationDtAccum; break;
        case RotationAxis::Y: sceneObject.mCustomRotation.y = mRotationDtAccum; break;
        case RotationAxis::Z: sceneObject.mCustomRotation.z = mRotationDtAccum; break;
    }
}

resources::ResourceId RotationAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId RotationAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId RotationAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

float RotationAnimation::VGetDuration() const
{
    return math::Abs(mRotationRadians)/mRotationSpeed;
}

///------------------------------------------------------------------------------------------------
