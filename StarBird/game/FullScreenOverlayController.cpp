///------------------------------------------------------------------------------------------------
///  FullScreenOverlayController.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FullScreenOverlayController.h"
#include "GameConstants.h"
#include "Scene.h"
#include "../resloading/ResourceLoadingService.h"

///------------------------------------------------------------------------------------------------

static const glm::vec3 FULL_SCREEN_OVERLAY_POSITION = glm::vec3(0.0f, 0.0f, 3.0f);
static const glm::vec3 FULL_SCREEN_OVERLAY_SCALE = glm::vec3(200.0f, 200.0f, 1.0f);

///------------------------------------------------------------------------------------------------

FullScreenOverlayController::FullScreenOverlayController(Scene& scene, const float darkeningSpeed, const float maxDarkeningValue, const bool pauseAtMidPoint, CallbackType midwayCallback /* nullptr */, CallbackType completionCallback /* nullptr */)
    : mScene(scene)
    , mDarkeningSpeed(darkeningSpeed)
    , mMaxDarkeningValue(maxDarkeningValue)
    , mPauseAtMidPoint(pauseAtMidPoint)
    , mDarkeningValue(0.0f)
    , mMidwayCallback(midwayCallback)
    , mCompletionCallback(completionCallback)
    , mDarkening(true)
    , mFinished(false)
    , mPaused(false)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    SceneObject overlaySo;
    overlaySo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
    overlaySo.mScale = FULL_SCREEN_OVERLAY_SCALE;
    overlaySo.mPosition = FULL_SCREEN_OVERLAY_POSITION;
    overlaySo.mName = game_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME;
    overlaySo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    overlaySo.mCrossSceneLifetime = true;
    mScene.AddSceneObject(std::move(overlaySo));
}

///------------------------------------------------------------------------------------------------

void FullScreenOverlayController::Update(const float dtMillis)
{
    if (mDarkening)
    {
        mDarkeningValue += dtMillis * mDarkeningSpeed;
        
        if (mDarkeningValue > mMaxDarkeningValue)
        {
            mDarkeningValue = mMaxDarkeningValue;
            mDarkening = false;
            
            if (mPauseAtMidPoint)
            {
                mPaused = true;
            }
            
            if (mMidwayCallback != nullptr)
            {
                mMidwayCallback();
                mMidwayCallback = nullptr;
            }
        }
    }
    else if (!mPaused)
    {
        mDarkeningValue -= dtMillis * mDarkeningSpeed;
        
        if (mDarkeningValue <= 0.0f)
        {
            mDarkeningValue = 0.0f;
            mFinished = true;
            
            if (mCompletionCallback != nullptr)
            {
                mCompletionCallback();
                mCompletionCallback = nullptr;
            }
        }
    }
    
    
    auto overlaySceneObjectOpt = mScene.GetSceneObject(game_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME);
    if (overlaySceneObjectOpt)
    {
        overlaySceneObjectOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = mDarkeningValue;
    }
}

///------------------------------------------------------------------------------------------------

void FullScreenOverlayController::Resume()
{
    mPaused = false;
}

///------------------------------------------------------------------------------------------------

bool FullScreenOverlayController::IsFinished() const
{
    return mFinished;
}

///------------------------------------------------------------------------------------------------
