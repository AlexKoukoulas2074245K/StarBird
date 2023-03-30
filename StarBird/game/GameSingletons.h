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
#include "Map.h"
#include "SceneObject.h"
#include "UpgradeDefinition.h"
#include "../utils/MathUtils.h"
#include "../utils/StringUtils.h"

#include <optional>
#include <map>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

struct SDL_Window;
class GameSingletons
{
public:
    static const InputContext& GetInputContext();
    static void SetInputContextEvent(Uint32 event);
    static void SetInputContextTouchPos(const glm::vec2& touchPos);
    static void SetInputContextText(const std::string& text);
    static void SetInputContextKey(const SDL_Scancode keyCode);
    static void SetInputContextPinchDistance(const float pinchDistance);
    static void SetInputContextMultiGestureActive(const bool multiGestureActive);
    static void ConsumeInput();
    
    static SDL_Window* GetWindow();
    static void SetWindow(SDL_Window* window);
    
    static const glm::vec2& GetWindowDimensions();
    static void SetWindowDimensions(const int windowWidth, const int windowHeight);
    
    static std::optional<std::reference_wrapper<Camera>> GetCameraForSceneObjectType(const SceneObjectType sceneObjectType);
    static void SetCameraForSceneObjectType(const SceneObjectType sceneObjectType, Camera&& camera);
    
    using UpgradeMap = std::map<strutils::StringId, UpgradeDefinition>;
    static UpgradeMap& GetEquippedUpgrades();
    static UpgradeMap& GetAvailableUpgrades();
    static void SetEquippedUpgrades(UpgradeMap&& upgrades);
    static void SetAvailableUpgrades(UpgradeMap&& upgrades);
    
    static std::pair<UpgradeDefinition, UpgradeDefinition>& GetUpgradeSelection();
    
    static float GetGameSpeedMultiplier();
    static void SetGameSpeedMultiplier(const float gameSpeedMultiplier);
    
    static float GetBossMaxHealth();
    static void SetBossMaxHealth(const float bossMaxHealth);
    
    static float GetBossCurrentHealth();
    static void SetBossCurrentHealth(const float bossCurrentHealth);
    
    static MapCoord GetCurrentMapCoord();
    static void SetCurrentMapCoord(const MapCoord& mapCoord);
    
    static const std::map<MapCoord, Map::NodeData>& GetMapData();
    static void SetMapData(const std::map<MapCoord, Map::NodeData>& mapData);
    
private:
    static InputContext mInputContext;
    static SDL_Window* mWindow;
    static glm::vec2 mWindowDimensions;
    static std::unordered_map<SceneObjectType, Camera> mSceneObjectTypeToCameraMap;
    static std::map<MapCoord, Map::NodeData> mMapData;
    static UpgradeMap mEquippedUpgrades;
    static UpgradeMap mAvailableUpgrades;
    static std::pair<UpgradeDefinition, UpgradeDefinition> mUpgradeSelection;
    static MapCoord mCurrentMapCoord;
    static float mGameSpeedMultiplier;
    static float mBossMaxHealth;
    static float mBossCurrentHealth;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSingletons_h */
