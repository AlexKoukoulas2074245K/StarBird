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
    enum class CharsAnchorMode
    {
        TOP_ANCHORED, BOT_ANCHORED
    };
    
    TextPromptController(Scene& scene, const glm::vec3& charsAnchorOrigin, const glm::vec3& scale, const CharsAnchorMode charsAnchorMode, const bool fadeIn, const std::string& text, std::function<void()> onFadeInCompleteCallback = nullptr);
    ~TextPromptController();
    
    void Update(const float dtMillis);
    
private:
    Scene& mScene;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mSceneObjectNamesToTransparencyDelayMillis;
    std::function<void()> mOnFadeInCompletionCallback;
};

///------------------------------------------------------------------------------------------------

#endif /* TextPromptController_h */
