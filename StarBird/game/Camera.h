///------------------------------------------------------------------------------------------------
///  Camera.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Camera_h
#define Camera_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"

///------------------------------------------------------------------------------------------------

class Camera final
{
public:
    Camera() = default;
    Camera(float cameraLenseHeight);
    
    void RecalculateMatrices();
    
    const float GetZoomFactor() const;
    const float GetCameraLenseWidth() const;
    const float GetCameraLenseHeight() const;
    const glm::vec3& GetPosition() const;
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjMatrix() const;
    
    void SetZoomFactor(const float zoomFactor);
    void SetPosition(const glm::vec3& position);
    
private:
    static const glm::vec3 DEFAULT_CAMERA_POSITION;
    static const glm::vec3 DEFAULT_CAMERA_FRONT_VECTOR;
    static const glm::vec3 DEFAULT_CAMERA_UP_VECTOR;
    static const float DEFAULT_CAMERA_ZNEAR;
    static const float DEFAULT_CAMERA_ZFAR;
    static const float DEFAULT_CAMERA_ZOOM_FACTOR;
    
private:
    float mZoomFactor;
    float mCameraLenseWidth;
    float mCameraLenseHeight;
    glm::vec3 mPosition;
    glm::mat4 mView;
    glm::mat4 mProj;
};

///------------------------------------------------------------------------------------------------

#endif /* Camera_h */
