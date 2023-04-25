///------------------------------------------------------------------------------------------------
///  Animations.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/02/2023
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "SceneObject.h"
#include "Scene.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"

#include <SDL.h>

///------------------------------------------------------------------------------------------------

BaseAnimation::BaseAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled)
    : mTextureResourceId(textureResourceId)
    , mMeshResourceId(meshResourceId)
    , mShaderResourceId(shaderResourceId)
    , mScale(scale)
    , mBodyRenderingEnabled(bodyRenderingEnabled)
    , mPaused(false)
{
}

void BaseAnimation::VUpdate(const float, SceneObject&)
{
}

bool BaseAnimation::VIsPaused() const
{
    return mPaused;
}

void BaseAnimation::VPause()
{
    mPaused = true;
}

void BaseAnimation::VResume()
{
    mPaused = false;
}

resources::ResourceId BaseAnimation::VGetCurrentTextureResourceId() const
{
    return mTextureResourceId;
}

resources::ResourceId BaseAnimation::VGetCurrentEffectTextureResourceId() const
{
    return 0;
}

resources::ResourceId BaseAnimation::VGetCurrentMeshResourceId() const
{
    return mMeshResourceId;
}

resources::ResourceId BaseAnimation::VGetCurrentShaderResourceId() const
{
    return mShaderResourceId;
}

const glm::vec3& BaseAnimation::VGetScale() const
{
    return mScale;
}

float BaseAnimation::VGetDurationMillis() const
{
    return 0.0f;
}

bool BaseAnimation::VGetBodyRenderingEnabled() const
{
    return mBodyRenderingEnabled;
}

void BaseAnimation::ChangeShaderResourceId(const resources::ResourceId shaderResourceId)
{
    mShaderResourceId = shaderResourceId;
}

void BaseAnimation::SetCompletionCallback(std::function<void()> completionCallback)
{
    mCompletionCallback = completionCallback;
}


///------------------------------------------------------------------------------------------------

SingleFrameAnimation::SingleFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
{
}

std::unique_ptr<BaseAnimation> SingleFrameAnimation::VClone() const
{
    return std::make_unique<SingleFrameAnimation>(*this);
}


///------------------------------------------------------------------------------------------------

SingleFrameAnimationWithEffectTexture::SingleFrameAnimationWithEffectTexture(const resources::ResourceId textureResourceId, const resources::ResourceId effectTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mEffectTextureResourceId(effectTextureResourceId)
{
}

std::unique_ptr<BaseAnimation> SingleFrameAnimationWithEffectTexture::VClone() const
{
    return std::make_unique<SingleFrameAnimationWithEffectTexture>(*this);
}

resources::ResourceId SingleFrameAnimationWithEffectTexture::VGetCurrentEffectTextureResourceId() const
{
    return mEffectTextureResourceId;
}

///------------------------------------------------------------------------------------------------

MultiFrameAnimation::MultiFrameAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float duration, const int textureSheetRow, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mDuration(duration)
    , mAnimationTime(0.0f)
    , mTextureSheetRow(textureSheetRow)
    , mAnimationIndex(0)
{
}

std::unique_ptr<BaseAnimation> MultiFrameAnimation::VClone() const
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
    
    sceneObject.mShaderBoolUniformValues[game_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
    sceneObject.mShaderFloatUniformValues[game_constants::MIN_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).minU;
    sceneObject.mShaderFloatUniformValues[game_constants::MIN_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).minV;
    sceneObject.mShaderFloatUniformValues[game_constants::MAX_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).maxU;
    sceneObject.mShaderFloatUniformValues[game_constants::MAX_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(mAnimationIndex).maxV;
}

float MultiFrameAnimation::VGetDurationMillis() const
{
    return mDuration;
}

///------------------------------------------------------------------------------------------------

VariableTexturedAnimation::VariableTexturedAnimation(const std::vector<resources::ResourceId>& potentialTextureResourceIds, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled)
    : BaseAnimation(potentialTextureResourceIds.at(math::RandomInt(0, static_cast<int>(potentialTextureResourceIds.size()) - 1)), meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mPotentialTextureResourceIds(potentialTextureResourceIds)
{
}
           
std::unique_ptr<BaseAnimation> VariableTexturedAnimation::VClone() const
{
    return std::make_unique<VariableTexturedAnimation>(mPotentialTextureResourceIds, mMeshResourceId, mShaderResourceId, mScale, mBodyRenderingEnabled);
}

///------------------------------------------------------------------------------------------------

PulsingAnimation::PulsingAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const PulsingMode pulsingMode, const float delayedStartMillis, const float pulsingSpeed, const float pulsingEnlargementFactor, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mPulsingMode(pulsingMode)
    , mDelayedStartMillis(delayedStartMillis)
    , mPulsingSpeed(pulsingSpeed)
    , mPulsingEnlargementFactor(pulsingEnlargementFactor)
    , mPulsingDtAccum(0.0f)
    , mCapturedOriginalScale(false)
    , mSignHasBeenReversed(false)
{
}

std::unique_ptr<BaseAnimation> PulsingAnimation::VClone() const
{
    return std::make_unique<PulsingAnimation>(*this);
}

void PulsingAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    if (!mCapturedOriginalScale)
    {
        mOriginalScale = sceneObject.mScale;
        mCapturedOriginalScale = true;
    }
    
    if ((mPulsingMode == PulsingMode::OUTER_PULSE_ONCE && math::Sinf(mPulsingDtAccum) < 0.0f) ||
        (mPulsingMode == PulsingMode::INNER_PULSE_ONCE && math::Sinf(mPulsingDtAccum) > 0.0f))
    {
        mSignHasBeenReversed = true;
    }
    
    if ((math::Sinf(mPulsingDtAccum) > 0.0f && mSignHasBeenReversed && mPulsingMode == PulsingMode::OUTER_PULSE_ONCE) ||
        (math::Sinf(mPulsingDtAccum) < 0.0f && mSignHasBeenReversed && mPulsingMode == PulsingMode::INNER_PULSE_ONCE))
    {
        sceneObject.mScale = mOriginalScale;
        
        BaseAnimation::VPause();
        if (mCompletionCallback)
        {
            mCompletionCallback();
        }
        
        return;
    }
    
    if (mDelayedStartMillis > 0.0f)
    {
        mDelayedStartMillis -= dtMillis;
    }
    else
    {
        mDelayedStartMillis = 0.0f;
        
        if (mPulsingMode == PulsingMode::INNER_PULSE_ONCE)
        {
            mPulsingDtAccum -= dtMillis * mPulsingSpeed;
        }
        else
        {
            mPulsingDtAccum += dtMillis * mPulsingSpeed;
        }
        
        sceneObject.mScale += math::Sinf(mPulsingDtAccum) * mPulsingEnlargementFactor;
    }
}

float PulsingAnimation::VGetDurationMillis() const
{
    return mPulsingEnlargementFactor;
}

///------------------------------------------------------------------------------------------------

BezierCurvePathAnimation::BezierCurvePathAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const math::BezierCurve& pathCurve, const float curveTraversalSpeed, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mPathCurve(pathCurve)
    , mCurveTraversalSpeed(curveTraversalSpeed)
    , mCurveTraversalProgress(0.0f)
{
}

std::unique_ptr<BaseAnimation> BezierCurvePathAnimation::VClone() const
{
    return std::make_unique<BezierCurvePathAnimation>(*this);
}

void BezierCurvePathAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    mCurveTraversalProgress += dtMillis * mCurveTraversalSpeed;
    sceneObject.mPosition = mPathCurve.ComputePointForT(mCurveTraversalProgress);
    
    if (mCurveTraversalProgress >= 1.0f)
    {
        BaseAnimation::VPause();
        if (mCompletionCallback)
        {
            mCompletionCallback();
        }
    }
}

float BezierCurvePathAnimation::VGetDurationMillis() const
{
    return mCurveTraversalSpeed;
}

float BezierCurvePathAnimation::VGetCurveTraversalProgress() const
{
    return mCurveTraversalProgress;
}

///------------------------------------------------------------------------------------------------

ShineAnimation::ShineAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId shineTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float shineSpeed, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mShineTextureResourceId(shineTextureResourceId)
    , mShineSpeed(shineSpeed)
    , mShineXOffset(game_constants::SHINE_EFFECT_X_OFFSET_INIT_VAL)
{
    if (sceneObject)
    {
        sceneObject->mShaderFloatUniformValues[game_constants::SHINE_X_OFFSET_UNIFORM_NAME] = mShineXOffset;
    }
}

std::unique_ptr<BaseAnimation> ShineAnimation::VClone() const
{
    return std::make_unique<ShineAnimation>(*this);
}

void ShineAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    mShineXOffset -= mShineSpeed * dtMillis;
    sceneObject.mShaderFloatUniformValues[game_constants::SHINE_X_OFFSET_UNIFORM_NAME] = mShineXOffset;
    
    if (mShineXOffset < -1.0f)
    {
        BaseAnimation::VPause();
        if (mCompletionCallback)
        {
            mCompletionCallback();
        }
    }
}

resources::ResourceId ShineAnimation::VGetCurrentEffectTextureResourceId() const
{
    return mShineTextureResourceId;
}

float ShineAnimation::VGetDurationMillis() const
{
    return math::Abs(game_constants::SHINE_EFFECT_X_OFFSET_END_VAL - game_constants::SHINE_EFFECT_X_OFFSET_INIT_VAL)/mShineSpeed;
}

///------------------------------------------------------------------------------------------------

DissolveAnimation::DissolveAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId dissolveTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float dissolveSpeed, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mDissolveTextureResourceId(dissolveTextureResourceId)
    , mDissolveSpeed(dissolveSpeed)
    , mDissolveYOffset(game_constants::DISSOLVE_EFFECT_Y_INIT_VAL)
{
    if (sceneObject)
    {
        sceneObject->mShaderFloatUniformValues[game_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] = mDissolveYOffset;
    }
}

std::unique_ptr<BaseAnimation> DissolveAnimation::VClone() const
{
    return std::make_unique<DissolveAnimation>(*this);
}

void DissolveAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    mDissolveYOffset -= mDissolveSpeed * dtMillis;
    sceneObject.mShaderFloatUniformValues[game_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] = mDissolveYOffset;
}

resources::ResourceId DissolveAnimation::VGetCurrentEffectTextureResourceId() const
{
    return mDissolveTextureResourceId;
}

float DissolveAnimation::VGetDurationMillis() const
{
    return game_constants::DISSOLVE_EFFECT_Y_INIT_VAL/mDissolveSpeed;
}

///------------------------------------------------------------------------------------------------

PlayerShieldAnimation::PlayerShieldAnimation(SceneObject* sceneObject, const resources::ResourceId textureResourceId, const resources::ResourceId alphaMapTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mAlphaMapTextureResourceId(alphaMapTextureResourceId)
{
    if (sceneObject)
    {
        sceneObject->mShaderFloatUniformValues[game_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] = 1.0f;
    }
    
    mDisolvingInProgress = false;
}

std::unique_ptr<BaseAnimation> PlayerShieldAnimation::VClone() const
{
    return std::make_unique<PlayerShieldAnimation>(*this);
}

void PlayerShieldAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    if (mDisolvingInProgress)
    {
        sceneObject.mShaderFloatUniformValues[game_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] -= dtMillis * 0.002f;
        
        if (sceneObject.mShaderFloatUniformValues[game_constants::DISSOLVE_Y_OFFSET_UNIFORM_NAME] <= -1.0f)
        {
            if (mCompletionCallback)
            {
                mCompletionCallback();
            }
        }
        
        sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * 0.001f;
        if (sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
        {
            sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        }
    }
    else
    {
        sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * 0.001f;
        if (sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
        {
            sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        }
    }
}

bool PlayerShieldAnimation::VIsPaused() const
{
    return false;
}

void PlayerShieldAnimation::VPause()
{
    mDisolvingInProgress = false;
}

void PlayerShieldAnimation::VResume()
{
    mDisolvingInProgress = true;
}

resources::ResourceId PlayerShieldAnimation::VGetCurrentEffectTextureResourceId() const
{
    return mAlphaMapTextureResourceId;
}

///------------------------------------------------------------------------------------------------

NebulaAnimation::NebulaAnimation(SceneObject* sceneObject, const resources::ResourceId noiseTextureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const float noiseMovementSpeed, const bool bodyRenderingEnabled)
    : BaseAnimation(noiseTextureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mNoiseMovementSpeed(noiseMovementSpeed)
{
    if (sceneObject)
    {
        sceneObject->mShaderFloatUniformValues[game_constants::TEXTURE_OFFSET_X_UNIFORM_NAME] = 0.0f;
    }
    
    mNoiseMovementDirection.x = math::RandomFloat(-1.0f, 1.0f);
    mNoiseMovementDirection.y = math::RandomFloat(-1.0f, 1.0f);
}


std::unique_ptr<BaseAnimation> NebulaAnimation::VClone() const
{
    return std::make_unique<NebulaAnimation>(*this);
}

void NebulaAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    sceneObject.mShaderFloatUniformValues[game_constants::TEXTURE_OFFSET_X_UNIFORM_NAME] += dtMillis * mNoiseMovementDirection.x * game_constants::NEBULA_ANIMATION_SPEED;
    sceneObject.mShaderFloatUniformValues[game_constants::TEXTURE_OFFSET_Y_UNIFORM_NAME] += dtMillis * mNoiseMovementDirection.y * game_constants::NEBULA_ANIMATION_SPEED;
}

///------------------------------------------------------------------------------------------------

RotationAnimation::RotationAnimation(const resources::ResourceId textureResourceId, const resources::ResourceId meshResourceId, const resources::ResourceId shaderResourceId, const glm::vec3& scale, const RotationMode rotationMode, const RotationAxis rotationAxis, const float rotationDegrees, const float rotationSpeed, const bool bodyRenderingEnabled)
    : BaseAnimation(textureResourceId, meshResourceId, shaderResourceId, scale, bodyRenderingEnabled)
    , mRotationMode(rotationMode)
    , mRotationAxis(rotationAxis)
    , mRotationRadians(glm::radians(rotationDegrees))
    , mPreviousRotationRadians(0.0f)
    , mRotationSpeed(rotationSpeed)
    , mRotationDtAccum(0.0f)
    , mLeftHandRotation(rotationDegrees < 0.0f)
    , mFinishedRotationOnce(false)
{
}

void RotationAnimation::SetRotationMode(const RotationMode rotationMode)
{
    mRotationMode = rotationMode;
}

std::unique_ptr<BaseAnimation> RotationAnimation::VClone() const
{
    return std::make_unique<RotationAnimation>(*this);
}

void RotationAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    if (mLeftHandRotation)
    {
        mRotationDtAccum -= dtMillis * mRotationSpeed;
        if (mRotationMode != RotationMode::ROTATE_CONTINUALLY)
        {
            if (mRotationDtAccum < mRotationRadians)
            {
                mRotationDtAccum = mRotationRadians;
                OnSingleRotationFinished();
            }
        }
    }
    else
    {
        mRotationDtAccum += dtMillis * mRotationSpeed;
        if (mRotationMode != RotationMode::ROTATE_CONTINUALLY)
        {
            if (mRotationDtAccum > mRotationRadians)
            {
                mRotationDtAccum = mRotationRadians;
                OnSingleRotationFinished();
            }
        }
    }
    
    switch (mRotationAxis)
    {
        case RotationAxis::X: sceneObject.mRotation.x = mRotationDtAccum; break;
        case RotationAxis::Y: sceneObject.mRotation.y = mRotationDtAccum; break;
        case RotationAxis::Z: sceneObject.mRotation.z = mRotationDtAccum; break;
    }
}

float RotationAnimation::VGetDurationMillis() const
{
    return math::Abs(mRotationRadians)/mRotationSpeed * (mRotationMode == RotationMode::ROTATE_TO_TARGET_AND_BACK_ONCE ? 2.0f : 1.0f);
}

void RotationAnimation::OnSingleRotationFinished()
{
    if (mRotationMode == RotationMode::ROTATE_TO_TARGET_AND_BACK_ONCE && !mFinishedRotationOnce)
    {
        mLeftHandRotation = !mLeftHandRotation;
        mRotationRadians = mPreviousRotationRadians;
        mPreviousRotationRadians = mRotationDtAccum;
    }
    else if (mRotationMode == RotationMode::ROTATE_TO_TARGET_AND_BACK_CONTINUALLY)
    {
        mLeftHandRotation = !mLeftHandRotation;
        mRotationRadians = mPreviousRotationRadians;
        mPreviousRotationRadians = mRotationDtAccum;
    }
    
    mFinishedRotationOnce = true;
}

///------------------------------------------------------------------------------------------------

static const int HEALTH_PARTICLES_COUNT = 18;

static const float HEALTH_PARTICLE_OFFSET_MIN = -2.0f;
static const float HEALTH_PARTICLE_OFFSET_MAX = 2.0f;
static const float HEALTH_PARTICLES_STAGGER_DELAY_MILLIS = 150.0f;
static const float HEALTH_PARTICLES_SPEED = 1.0f/900.0f;
static const glm::vec3 HEALTH_PARTICLE_SCALE = glm::vec3(1.2f, 1.2f, 1.2f);

static const char* HEALTH_PARTICLE_NAME_PREFIX = "HEALTH_PARTICLE_";


HealthUpParticlesAnimation::HealthUpParticlesAnimation(Scene& scene, const glm::vec3& originPosition)
    : BaseAnimation(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::HEALTH_PARTICLE_TEXTURE_FILE_NAME), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false)
    , mScene(scene)
    , mOriginPosition(originPosition)
{
    
    
    for (int i = 0; i < HEALTH_PARTICLES_COUNT; ++i)
    {
        mFlows.emplace_back([this]()
        {
            SceneObject particleSo;
            
            glm::vec3 firstControlPoint(mOriginPosition + glm::vec3(math::RandomFloat(HEALTH_PARTICLE_OFFSET_MIN, HEALTH_PARTICLE_OFFSET_MAX), 0.0f, 0.0f));
            glm::vec3 secondControlPoint(firstControlPoint + glm::vec3(0.0f, 4.0f, 0.0f));
            
            firstControlPoint.z = mOriginPosition.z + 0.5f;
            secondControlPoint.z = mOriginPosition.z + 0.5f;
        
            const strutils::StringId healthParticleName = strutils::StringId(HEALTH_PARTICLE_NAME_PREFIX + std::to_string(SDL_GetPerformanceCounter()));
            
            particleSo.mAnimation = std::make_unique<BezierCurvePathAnimation>(VGetCurrentTextureResourceId(), VGetCurrentMeshResourceId(), VGetCurrentShaderResourceId(), glm::vec3(1.0f), math::BezierCurve({firstControlPoint, secondControlPoint}), HEALTH_PARTICLES_SPEED, VGetBodyRenderingEnabled());
            particleSo.mAnimation->SetCompletionCallback([healthParticleName, this]()
            {
                mHealthParticleNamesToAlphaRadValue.erase(healthParticleName);
                mScene.RemoveAllSceneObjectsWithName(healthParticleName);
            });
            
            particleSo.mSceneObjectType = SceneObjectType::GUIObject;
            particleSo.mPosition = firstControlPoint;
            particleSo.mScale = HEALTH_PARTICLE_SCALE;
            particleSo.mName = healthParticleName;
            particleSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            mHealthParticleNamesToAlphaRadValue[particleSo.mName] = 0.0f;
            mScene.AddSceneObject(std::move(particleSo));
        }, i * HEALTH_PARTICLES_STAGGER_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE);
    }
}

std::unique_ptr<BaseAnimation> HealthUpParticlesAnimation::VClone() const
{
    return std::make_unique<HealthUpParticlesAnimation>(*this);
}

void HealthUpParticlesAnimation::VUpdate(const float dtMillis, SceneObject& sceneObject)
{
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
    
    if (mHealthParticleNamesToAlphaRadValue.empty())
    {
        VPause();
        if (mCompletionCallback)
        {
            mCompletionCallback();
        }
    }
    for (auto& healthParticleEntry: mHealthParticleNamesToAlphaRadValue)
    {
        auto healthParticleSoOpt = mScene.GetSceneObject(healthParticleEntry.first);
        if (healthParticleSoOpt)
        {
            auto& healthParticleSo = healthParticleSoOpt->get();
            const auto healthParticleBezierProgress = dynamic_cast<BezierCurvePathAnimation*>(healthParticleSo.mAnimation.get())->VGetCurveTraversalProgress();
            healthParticleSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Abs(math::Sinf(healthParticleBezierProgress * math::PI))/2.0f;
        }
    }
}

float HealthUpParticlesAnimation::VGetDurationMillis() const
{
    static const float ANIMATION_DURATION = 4000.0f;
    return ANIMATION_DURATION;
}

///------------------------------------------------------------------------------------------------
