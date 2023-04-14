///------------------------------------------------------------------------------------------------
///  CarouselController.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "CarouselController.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "SceneObject.h"
#include "Scene.h"

///------------------------------------------------------------------------------------------------

static const float CAROUSEL_OBJECT_X_MULTIPLIER = 4.2f;
static const float CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT = 3.5f;
static const float CAROUSEL_ROTATION_THRESHOLD = 0.5f;
static const float CAROUSEL_ROTATION_SPEED = 0.006f;

///------------------------------------------------------------------------------------------------

CarouselController::CarouselController(Scene& scene, const std::vector<resources::ResourceId>& carouselEntryTextures, std::function<void()> onCarouselMovementStartCallback /* = nullptr */, std::function<void()> onCarouselStationaryCallback /* = nullptr */, const float baseCarouselEntryZ /* = 2.0f */)
    : mScene(scene)
    , mCarouselEntries(carouselEntryTextures)
    , mOnCarouselMovementStartCallback(onCarouselMovementStartCallback)
    , mOnCarouselStationaryCallback(onCarouselStationaryCallback)
    , mCarouselState(CarouselState::STATIONARY)
    , mBaseCarouselEntryZ(baseCarouselEntryZ)
    , mFingerDownPosition(0.0f)
    , mCarouselRads(0.0f)
    , mCarouselTargetRads(0.0f)
    , mSelectedEntryIndex(0)
    , mExhaustedMove(false)
    , mHasInvokedStationaryOnce(false)
{
    CreateSceneObjects();
}

///------------------------------------------------------------------------------------------------

void CarouselController::Update(const float dtMillis)
{
    // The first OnStationary should be done in the constructor, however in order to invoke bad access on the callback side asking
    // for the selected index before the constructor has even finished, we do it once on the first update
    if (!mHasInvokedStationaryOnce)
    {
        OnStationary();
        mHasInvokedStationaryOnce = true;
    }
    
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    
    // Input flow
    auto& inputContext = GameSingletons::GetInputContext();
    
    if (inputContext.mEventType == SDL_FINGERDOWN && !mExhaustedMove)
    {
        mFingerDownPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
    }
    else if (inputContext.mEventType == SDL_FINGERMOTION && !mExhaustedMove)
    {
        auto currentTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        if (mCarouselState == CarouselState::STATIONARY && math::Abs(mFingerDownPosition.x - currentTouchPos.x) > CAROUSEL_ROTATION_THRESHOLD)
        {
            if (currentTouchPos.x > mFingerDownPosition.x)
            {
                mCarouselState = CarouselState::MOVING_LEFT;
                mCarouselTargetRads = mCarouselRads + (math::PI * 2.0f / (mCarouselEntries.size()));
                
                if (mOnCarouselMovementStartCallback)
                {
                    mOnCarouselMovementStartCallback();
                }
            }
            else
            {
                mCarouselState = CarouselState::MOVING_RIGHT;
                mCarouselTargetRads = mCarouselRads - (math::PI * 2.0f / (mCarouselEntries.size()));
                
                if (mOnCarouselMovementStartCallback)
                {
                    mOnCarouselMovementStartCallback();
                }
            }
            
            mExhaustedMove = true;
        }
    }
    else if (inputContext.mEventType == SDL_FINGERUP)
    {
        mExhaustedMove = false;
    }
    
    // Rotate lab options
    if (mCarouselState == CarouselState::MOVING_LEFT)
    {
        mCarouselRads += dtMillis * CAROUSEL_ROTATION_SPEED;
        if (mCarouselRads >= mCarouselTargetRads)
        {
            mCarouselRads = mCarouselTargetRads;
            mCarouselState = CarouselState::STATIONARY;
            OnStationary();
        }
    }
    else if (mCarouselState == CarouselState::MOVING_RIGHT)
    {
        mCarouselRads -= dtMillis * CAROUSEL_ROTATION_SPEED;
        if (mCarouselRads <= mCarouselTargetRads)
        {
            mCarouselRads = mCarouselTargetRads;
            mCarouselState = CarouselState::STATIONARY;
            OnStationary();
        }
    }
    
    // Give fake perspective to all Lab options
    for (int i = 0; i < static_cast<int>(mCarouselEntries.size()); ++i)
    {
        auto labOptionSoOpt = mScene.GetSceneObject(strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i)));
        if (labOptionSoOpt)
        {
            auto& labOptionSo = labOptionSoOpt->get();
            PositionCarouselObject(labOptionSo, i);
        }
    }
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<SceneObject>> CarouselController::GetSelectedSceneObject() const
{
    return mScene.GetSceneObject(strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(mSelectedEntryIndex)));
}

///------------------------------------------------------------------------------------------------

int CarouselController::GetSelectedIndex() const
{
    return mSelectedEntryIndex;
}

///------------------------------------------------------------------------------------------------

void CarouselController::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    for (int i = 0; i < static_cast<int>(mCarouselEntries.size()); ++i)
    {
        SceneObject optionEntrySo;
        optionEntrySo.mAnimation = std::make_unique<SingleFrameAnimation>(mCarouselEntries.at(i), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::DARKENED_COLOR_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        optionEntrySo.mSceneObjectType = SceneObjectType::WorldGameObject;
        optionEntrySo.mName = strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i));
        optionEntrySo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        
        PositionCarouselObject(optionEntrySo, i);
        mScene.AddSceneObject(std::move(optionEntrySo));
    }
}

///------------------------------------------------------------------------------------------------

void CarouselController::OnStationary()
{
    if (mOnCarouselStationaryCallback)
    {
        auto closestZ = -1.0f;
        mSelectedEntryIndex = 0;
        
        for (int i = 0; i < static_cast<int>(mCarouselEntries.size()); ++i)
        {
            auto carouselEntrySoOpt = mScene.GetSceneObject(strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i)));
            
            if (carouselEntrySoOpt && carouselEntrySoOpt->get().mPosition.z > closestZ)
            {
                closestZ = carouselEntrySoOpt->get().mPosition.z;
                mSelectedEntryIndex = i;
            }
        }
        
        mOnCarouselStationaryCallback();
    }
}

///------------------------------------------------------------------------------------------------

void CarouselController::PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const
{
    float optionRadsOffset = objectIndex * (math::PI * 2.0f / mCarouselEntries.size());
    carouselObject.mPosition.x = math::Sinf(mCarouselRads + optionRadsOffset) * CAROUSEL_OBJECT_X_MULTIPLIER;
    carouselObject.mPosition.z = mBaseCarouselEntryZ + math::Cosf(mCarouselRads + optionRadsOffset);
    carouselObject.mScale = glm::vec3(carouselObject.mPosition.z + CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT, carouselObject.mPosition.z + CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT, 1.0f);
    
    carouselObject.mShaderFloatUniformValues[game_constants::DARKEN_VALUE_UNIFORM_NAME] = math::Max(((carouselObject.mPosition.z - mBaseCarouselEntryZ)/2.0f) + 0.5f, 0.0f);
}

///------------------------------------------------------------------------------------------------
