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
#include <unordered_map>
#include <vector>

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
    
    static std::vector<UpgradeDefinition>& GetEquippedUpgrades();
    static std::vector<UpgradeDefinition>& GetAvailableUpgrades();
    static void SetEquippedUpgrades(std::vector<UpgradeDefinition>&& upgrades);
    static void SetAvailableUpgrades(std::vector<UpgradeDefinition>&& upgrades);
    static bool HasEquippedUpgrade(const strutils::StringId& upgradeNameId);
    
    static float GetGameSpeedMultiplier();
    static void SetGameSpeedMultiplier(const float gameSpeedMultiplier);
    
    static float GetBossMaxHealth();
    static void SetBossMaxHealth(const float bossMaxHealth);
    
    static float GetBossCurrentHealth();
    static void SetBossCurrentHealth(const float bossCurrentHealth);
    
    static float GetPlayerMaxHealth();
    static void SetPlayerMaxHealth(const float playerMaxHealth);
    
    static float GetPlayerCurrentHealth();
    static void SetPlayerCurrentHealth(const float playerCurrentHealth);
    
    static float GetPlayerDisplayedHealth();
    static void SetPlayerDisplayedHealth(const float playerDisplayedHealth);
    
    static float GetPlayerAttackStat();
    static void SetPlayerAttackStat(const float playerAttackStat);
    
    static float GetPlayerBulletSpeedStat();
    static void SetPlayerBulletSpeedStat(const float playerBulletSpeedStat);
    
    static float GetPlayerMovementSpeedStat();
    static void SetPlayerMovementSpeedStat(const float playerMovementStat);
    
    static long GetCrystalCount();
    static void SetCrystalCount(const long crystalCount);
    
    static float GetDisplayedCrystalCount();
    static void SetDisplayedCrystalCount(const float displayedCrystalCount);
    
    static MapCoord GetCurrentMapCoord();
    static void SetCurrentMapCoord(const MapCoord& mapCoord);
    
    static int GetMapGenerationSeed();
    static void SetMapGenerationSeed(const int mapGenerationSeed);
    
private:
    static InputContext mInputContext;
    static SDL_Window* mWindow;
    static glm::vec2 mWindowDimensions;
    static std::unordered_map<SceneObjectType, Camera> mSceneObjectTypeToCameraMap;
    static std::vector<UpgradeDefinition> mEquippedUpgrades;
    static std::vector<UpgradeDefinition> mAvailableUpgrades;
    static MapCoord mCurrentMapCoord;
    static int mMapGenerationSeed;
    static long mCrystalCount;
    static float mDisplayedCrystalCount;
    static float mGameSpeedMultiplier;
    static float mBossMaxHealth;
    static float mBossCurrentHealth;
    static float mPlayerMaxHealth;
    static float mPlayerCurrentHealth;
    static float mPlayerDisplayedHealth;
    static float mPlayerAttackStat;
    static float mPlayerBulletSpeedStat;
    static float mPlayerMovementStat;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSingletons_h */
