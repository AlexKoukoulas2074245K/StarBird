///------------------------------------------------------------------------------------------------
///  Camera.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "GameSingletons.h"

///------------------------------------------------------------------------------------------------

const glm::vec3 Camera::DEFAULT_CAMERA_POSITION     = glm::vec3(0.0f, 0.0f, -5.0f);
const glm::vec3 Camera::DEFAULT_CAMERA_FRONT_VECTOR = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 Camera::DEFAULT_CAMERA_UP_VECTOR    = glm::vec3(0.0f, 1.0f, 0.0f);

const float Camera::DEVICE_INVARIABLE_ASPECT = 0.46f;

const float Camera::DEFAULT_CAMERA_ZNEAR       = -50.0f;
const float Camera::DEFAULT_CAMERA_ZFAR        = 50.0f;
const float Camera::DEFAULT_CAMERA_ZOOM_FACTOR = 16.0f/14.0f;

const float Camera::SHAKE_DAMPING    = 0.72f;
const float Camera::SHAKE_MAX_RADIUS = 0.5f;
const float Camera::SHAKE_MIN_RADIUS = 0.001f;

///------------------------------------------------------------------------------------------------

Camera::Camera(const float cameraLenseHeight)
    : mPosition(DEFAULT_CAMERA_POSITION)
    , mZoomFactor(DEFAULT_CAMERA_ZOOM_FACTOR)
{
    mCameraLenseWidth = cameraLenseHeight * DEVICE_INVARIABLE_ASPECT;
    mCameraLenseHeight = cameraLenseHeight;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------

void Camera::RecalculateMatrices()
{
    const auto& windowDimensions = GameSingletons::GetWindowDimensions();
    float aspect = windowDimensions.x/windowDimensions.y;
    mView = glm::lookAt(mPosition, mPosition + DEFAULT_CAMERA_FRONT_VECTOR, DEFAULT_CAMERA_UP_VECTOR);
    mProj = glm::ortho((-mCameraLenseWidth/(DEVICE_INVARIABLE_ASPECT/aspect))/2.0f/mZoomFactor, (mCameraLenseWidth/((DEVICE_INVARIABLE_ASPECT/aspect)))/2.0f/mZoomFactor, -mCameraLenseHeight/2.0f/mZoomFactor, mCameraLenseHeight/2.0f/mZoomFactor, DEFAULT_CAMERA_ZNEAR, DEFAULT_CAMERA_ZFAR);
}

///------------------------------------------------------------------------------------------------

const float Camera::GetZoomFactor() const
{
    return mZoomFactor;
}

///------------------------------------------------------------------------------------------------

const float Camera::GetCameraLenseWidth() const
{
    return mCameraLenseWidth;
}

///------------------------------------------------------------------------------------------------

const float Camera::GetCameraLenseHeight() const
{
    return mCameraLenseHeight;
}

///------------------------------------------------------------------------------------------------

const glm::vec3& Camera::GetPosition() const
{
    return mPosition;
}

///------------------------------------------------------------------------------------------------

const glm::mat4& Camera::GetViewMatrix() const
{
    return mView;
}

///------------------------------------------------------------------------------------------------

const glm::mat4& Camera::GetProjMatrix() const
{
    return mProj;
}

///------------------------------------------------------------------------------------------------

void Camera::Shake()
{
    if (mShakeData.mShakeRadius <= SHAKE_MIN_RADIUS)
    {
        mShakeData.mIsShaking = true;
        mShakeData.mPreShakePosition = mPosition;
        
        mShakeData.mShakeRadius = SHAKE_MAX_RADIUS;
        
        mShakeData.mShakeRandomAngle = math::RandomFloat(0.0f, 2.0f * math::PI);
        auto offset = glm::vec2(math::Sinf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeRadius, math::Cosf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeRadius);
        
        SetPosition(glm::vec3(mShakeData.mPreShakePosition.x + offset.x, mShakeData.mPreShakePosition.y + offset.y, GetPosition().z));
    }
}

///------------------------------------------------------------------------------------------------

void Camera::Update(const float dtMillis)
{
    if (mShakeData.mIsShaking)
    {
        mShakeData.mShakeRadius *= SHAKE_DAMPING;
        
        if (mShakeData.mShakeRadius <= SHAKE_MIN_RADIUS)
        {
            mShakeData.mIsShaking = false;
            mShakeData.mShakeRadius = SHAKE_MIN_RADIUS;
            SetPosition(mShakeData.mPreShakePosition);
        }
        else
        {
            mShakeData.mShakeRandomAngle = math::RandomFloat(0.0f, 2.0f * math::PI);
            auto offset = glm::vec2(math::Sinf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeRadius, math::Cosf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeRadius);
            
            SetPosition(glm::vec3(mShakeData.mPreShakePosition.x + offset.x, mShakeData.mPreShakePosition.y + offset.y, GetPosition().z));
        }
    }
}

///------------------------------------------------------------------------------------------------

void Camera::SetZoomFactor(const float zoomFactor)
{
    mZoomFactor = zoomFactor;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------

void Camera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------
