///------------------------------------------------------------------------------------------------
///  GameSingletons.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameSingletons_h
#define GameSingletons_h

///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "InputContext.h"
#include "SceneObject.h"
#include "../utils/MathUtils.h"

#include <optional>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

struct SDL_Window;
class GameSingletons
{
public:
    static const InputContext& GetInputContext();
    static void SetInputContextEvent(Uint32 event);
    static void SetInputContextTouchPos(const glm::vec2& touchPos);
    
    static SDL_Window* GetWindow();
    static void SetWindow(SDL_Window* window);
    
    static const glm::vec2& GetWindowDimensions();
    static void SetWindowDimensions(const int windowWidth, const int windowHeight);
    
    static std::optional<std::reference_wrapper<Camera>> GetCameraForSceneObjectType(const SceneObjectType sceneObjectType);
    static void SetCameraForSceneObjectType(const SceneObjectType sceneObjectType, Camera&& camera);
    
private:
    static InputContext mInputContext;
    static SDL_Window* mWindow;
    static glm::vec2 mWindowDimensions;
    static std::unordered_map<SceneObjectType, Camera> mSceneObjectTypeToCameraMap;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSingletons_h */
