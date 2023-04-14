///------------------------------------------------------------------------------------------------
///  CarouselController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef CarouselController_h
#define CarouselController_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../resloading/ResourceLoadingService.h"

#include <functional>
#include <string>
#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;
class SceneObject;
class CarouselController final
{
public:
    CarouselController(Scene& scene, const std::vector<resources::ResourceId>& carouselEntryTextures, std::function<void()> onCarouselMovementStartCallback = nullptr, std::function<void(const int)> onCarouselStationaryCallback = nullptr, const float baseCarouselEntryZ = 2.0f);
    
    void Update(const float dtMillis);

private:
    void CreateSceneObjects();
    void OnStationary();
    void PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const;
    
private:
    enum class CarouselState
    {
        STATIONARY, MOVING_LEFT, MOVING_RIGHT
    };
    
private:
    Scene& mScene;
    std::vector<resources::ResourceId> mCarouselEntries;
    std::function<void()> mOnCarouselMovementStartCallback;
    std::function<void(const int)> mOnCarouselStationaryCallback;
    CarouselState mCarouselState;
    glm::vec3 mFingerDownPosition;
    const float mBaseCarouselEntryZ;
    float mCarouselRads;
    float mCarouselTargetRads;
    bool mExhaustedMove;
};

///------------------------------------------------------------------------------------------------

#endif /* CarouselController_h */
