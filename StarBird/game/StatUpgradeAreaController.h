///------------------------------------------------------------------------------------------------
///  StatUpgradeAreaController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef StatUpgradeAreaController_h
#define StatUpgradeAreaController_h

///------------------------------------------------------------------------------------------------

#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include <memory>
#include <string>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class BaseAnimation;
class Scene;
class SceneObject;
class StatUpgradeAreaController final
{
public:
    StatUpgradeAreaController(Scene& scene, std::unique_ptr<BaseAnimation> statUpgradeBackgroundAnimation, const glm::vec3& position, const glm::vec3& scale, const std::string& text, const float initialStatValue);
    
    void Update(const float dtMillis);
    
private:
    Scene& mScene;
    float mStatValue;
};

///------------------------------------------------------------------------------------------------

#endif /* StatUpgradeAreaController_h */
