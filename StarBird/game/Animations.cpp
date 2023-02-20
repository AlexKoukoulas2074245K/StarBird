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

SingleFrameAnimation::SingleFrameAnimation(const resources::ResourceId textureResourceId)
    : mTextureResourceId(textureResourceId)
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

float SingleFrameAnimation::VGetDuration() const
{
    return 0.0f;
}

///------------------------------------------------------------------------------------------------

MultiFrameAnimation::MultiFrameAnimation(const resources::ResourceId textureResourceId, const float duration, const float scale, const int textureSheetRow)
    : mTextureResourceId(textureResourceId)
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

float MultiFrameAnimation::VGetDuration() const
{
    return mDuration;
}

///------------------------------------------------------------------------------------------------

VariableTexturedAnimation::VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds)
    : mTextureResourceId(potentialTextureResourceIds.at(math::RandomInt(0, static_cast<int>(potentialTextureResourceIds.size()) - 1)))
    , mPotentialTextureResourceIds(potentialTextureResourceIds)
{
}
           
std::unique_ptr<IAnimation> VariableTexturedAnimation::VClone() const
{
    return std::make_unique<VariableTexturedAnimation>(mPotentialTextureResourceIds);
}

void VariableTexturedAnimation::VUpdate(const float, SceneObject& sceneObject)
{
}

resources::ResourceId VariableTexturedAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

float VariableTexturedAnimation::VGetDuration() const
{
    return 0.0f;
}

///------------------------------------------------------------------------------------------------

PulsingAnimation::PulsingAnimation(const resources::ResourceId textureResourceId, const float pulsingSpeed, const float pulsingEnlargementFactor)
    : mTextureResourceId(textureResourceId)
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

float PulsingAnimation::VGetDuration() const
{
    return mPulsingEnlargementFactor;
}

///------------------------------------------------------------------------------------------------

ShineAnimation::ShineAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const float shineSpeed)
    : mTextureResourceId(textureResourceId)
    , mShineTextureResourceId(shineTextureResourceId)
    , mShineSpeed(shineSpeed)
    , mShineXOffset(scene_object_constants::SHINE_EFFECT_X_OFFSET_INIT_VAL)
    , mShineShaderResourceId(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::SHINE_SHADER_FILE_NAME))
{
   
}

std::unique_ptr<IAnimation> ShineAnimation::VClone() const
{
    return std::make_unique<ShineAnimation>(*this);
}

void ShineAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    sceneObject.mShaderResourceId = mShineShaderResourceId;
    sceneObject.mShaderEffectTextureResourceId = mShineTextureResourceId;
    mShineXOffset -= mShineSpeed * dtMillis;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::SHINE_X_OFFSET_UNIFORM_NAME] = mShineXOffset;
}

resources::ResourceId ShineAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

float ShineAnimation::VGetDuration() const
{
    return math::Abs(scene_object_constants::SHINE_EFFECT_X_OFFSET_END_VAL - scene_object_constants::SHINE_EFFECT_X_OFFSET_INIT_VAL)/mShineSpeed;
}

///------------------------------------------------------------------------------------------------

DissolveAnimation::DissolveAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const float dissolveSpeed)
    : mTextureResourceId(textureResourceId)
    , mDissolveTextureResourceId(dissolveTextureResourceId)
    , mDissolveSpeed(dissolveSpeed)
    , mDissolveYOffset(scene_object_constants::DISSOLVE_EFFECT_Y_INIT_VAL)
    , mDissolveShaderResourceId(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::DISSOLVE_SHADER_FILE_NAME))
{
}

std::unique_ptr<IAnimation> DissolveAnimation::VClone() const
{
    return std::make_unique<DissolveAnimation>(*this);
}

void DissolveAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    sceneObject.mShaderResourceId = mDissolveShaderResourceId;
    sceneObject.mShaderEffectTextureResourceId = mDissolveTextureResourceId;
    mDissolveYOffset -= mDissolveSpeed * dtMillis;
    sceneObject.mShaderFloatUniformValues[scene_object_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] = mDissolveYOffset;
}

resources::ResourceId DissolveAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

float DissolveAnimation::VGetDuration() const
{
    return scene_object_constants::DISSOLVE_EFFECT_Y_INIT_VAL/mDissolveSpeed;
}

///------------------------------------------------------------------------------------------------
