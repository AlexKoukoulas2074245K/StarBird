///------------------------------------------------------------------------------------------------
///  Camera.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Camera.h"

///------------------------------------------------------------------------------------------------

const glm::vec3 Camera::DEFAULT_CAMERA_POSITION     = glm::vec3(0.0f, 0.0f, 5.0f);
const glm::vec3 Camera::DEFAULT_CAMERA_FRONT_VECTOR = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 Camera::DEFAULT_CAMERA_UP_VECTOR    = glm::vec3(0.0f, 1.0f, 0.0f);

const float Camera::DEFAULT_CAMERA_ZNEAR       = 0.0f;
const float Camera::DEFAULT_CAMERA_ZFAR        = 50.0f;
const float Camera::DEFAULT_CAMERA_ZOOM_FACTOR = 16.0f/14.0f;

///------------------------------------------------------------------------------------------------

Camera::Camera(float windowWidth, float windowHeight, float cameraLenseHeight)
    : mPosition(DEFAULT_CAMERA_POSITION)
    , mZoomFactor(DEFAULT_CAMERA_ZOOM_FACTOR)
{
    float aspect = windowWidth/windowHeight;
    mCameraLenseWidth = cameraLenseHeight * aspect;
    mCameraLenseHeight = cameraLenseHeight;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------

void Camera::RecalculateMatrices()
{
    mView = glm::lookAt(DEFAULT_CAMERA_POSITION, DEFAULT_CAMERA_POSITION + DEFAULT_CAMERA_FRONT_VECTOR, DEFAULT_CAMERA_UP_VECTOR);
    mProj = glm::ortho(-mCameraLenseWidth/2.0f/mZoomFactor, mCameraLenseWidth/2.0f/mZoomFactor, -mCameraLenseHeight/2.0f/mZoomFactor, mCameraLenseHeight/2.0f/mZoomFactor, DEFAULT_CAMERA_ZNEAR, DEFAULT_CAMERA_ZFAR);
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
