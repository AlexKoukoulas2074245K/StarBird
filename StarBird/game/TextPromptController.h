///------------------------------------------------------------------------------------------------
///  TextPromptController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef TextPromptController_h
#define TextPromptController_h

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
class TextPromptController final
{
public:
    TextPromptController(Scene& scene, std::unique_ptr<BaseAnimation> animation, const glm::vec3& position, const glm::vec3& scale, bool fadeIn, const std::string& text);
    ~TextPromptController();
    
    void Update(const float dtMillis);
    
private:
    Scene& mScene;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mSceneObjectNamesToTransparencyDelayMillis;
};

///------------------------------------------------------------------------------------------------

#endif /* TextPromptController_h */
