///------------------------------------------------------------------------------------------------
///  GameSingletons.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "GameSingletons.h"

///------------------------------------------------------------------------------------------------

InputContext GameSingletons::mInputContext = {};
SDL_Window* GameSingletons::mWindow = nullptr;
glm::vec2 GameSingletons::mWindowDimensions = glm::vec2();
std::unordered_map<SceneObjectType, Camera> GameSingletons::mSceneObjectTypeToCameraMap = {};
std::map<MapCoord, Map::NodeData> GameSingletons::mMapData = {};
GameSingletons::UpgradeMap GameSingletons::mEquippedUpgrades = {};
GameSingletons::UpgradeMap GameSingletons::mAvailableUpgrades = {};
std::pair<UpgradeDefinition, UpgradeDefinition> GameSingletons::mUpgradeSelection = {};
MapCoord GameSingletons::mCurrentMapCoord = MapCoord(0, 2);
long GameSingletons::mCrystalCount = 0;
float GameSingletons::mDisplayedCrystalCount = 0;
float GameSingletons::mGameSpeedMultiplier = 1.0f;
float GameSingletons::mBossMaxHealth = 0.0f;
float GameSingletons::mBossCurrentHealth = 1.0f;
float GameSingletons::mPlayerMaxHealth = 1.0f;
float GameSingletons::mPlayerCurrentHealth = 1.0f;
float GameSingletons::mPlayerDisplayedHealth = 1.0f;

///------------------------------------------------------------------------------------------------

const InputContext& GameSingletons::GetInputContext()
{
    return mInputContext;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetInputContextEvent(Uint32 event)
{
    mInputContext.mEventType = event;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetInputContextTouchPos(const glm::vec2& touchPos)
{
    mInputContext.mTouchPos = touchPos;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetInputContextText(const std::string& text)
{
    mInputContext.mText = text;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetInputContextKey(const SDL_Scancode keyCode)
{
    mInputContext.mKeyCode = keyCode;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetInputContextPinchDistance(const float pinchDistance)
{
    mInputContext.mPinchDistance = pinchDistance;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetInputContextMultiGestureActive(const bool multiGestureActive)
{
    mInputContext.mMultiGestureActive = multiGestureActive;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::ConsumeInput()
{
    SetInputContextEvent(SDL_FINGERUP);
}

///------------------------------------------------------------------------------------------------

SDL_Window* GameSingletons::GetWindow()
{
    return mWindow;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetWindow(SDL_Window* window)
{
    mWindow = window;
}

///------------------------------------------------------------------------------------------------

const glm::vec2& GameSingletons::GetWindowDimensions()
{
    return mWindowDimensions;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetWindowDimensions(int windowWidth, int windowHeight)
{
    mWindowDimensions.x = windowWidth;
    mWindowDimensions.y = windowHeight;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<Camera>> GameSingletons::GetCameraForSceneObjectType(const SceneObjectType sceneObjectType)
{
    auto findIter = mSceneObjectTypeToCameraMap.find(sceneObjectType);
    if (findIter != mSceneObjectTypeToCameraMap.end())
    {
        return std::optional<std::reference_wrapper<Camera>>{findIter->second};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetCameraForSceneObjectType(const SceneObjectType sceneObjectType, Camera&& camera)
{
    mSceneObjectTypeToCameraMap[sceneObjectType] = camera;
}

///------------------------------------------------------------------------------------------------

GameSingletons::UpgradeMap& GameSingletons::GetEquippedUpgrades()
{
    return mEquippedUpgrades;
}

///------------------------------------------------------------------------------------------------

GameSingletons::UpgradeMap& GameSingletons::GetAvailableUpgrades()
{
    return mAvailableUpgrades;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetEquippedUpgrades(UpgradeMap&& upgrades)
{
    mEquippedUpgrades = upgrades;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetAvailableUpgrades(UpgradeMap&& upgrades)
{
    mAvailableUpgrades = upgrades;
}

///------------------------------------------------------------------------------------------------

std::pair<UpgradeDefinition, UpgradeDefinition>& GameSingletons::GetUpgradeSelection()
{
    return mUpgradeSelection;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetGameSpeedMultiplier()
{
    return mGameSpeedMultiplier;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetGameSpeedMultiplier(const float gameSpeedMultiplier)
{
    mGameSpeedMultiplier = gameSpeedMultiplier;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetBossMaxHealth()
{
    return mBossMaxHealth;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetBossMaxHealth(const float bossMaxHealth)
{
    mBossMaxHealth = bossMaxHealth;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetBossCurrentHealth()
{
    return mBossCurrentHealth;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetBossCurrentHealth(const float bossCurrentHealth)
{
    mBossCurrentHealth = bossCurrentHealth;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetPlayerMaxHealth()
{
    return mPlayerMaxHealth;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetPlayerMaxHealth(const float playerMaxHealth)
{
    mPlayerMaxHealth = playerMaxHealth;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetPlayerCurrentHealth()
{
    return mPlayerCurrentHealth;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetPlayerCurrentHealth(const float playerCurrentHealth)
{
    mPlayerCurrentHealth = playerCurrentHealth;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetPlayerDisplayedHealth()
{
    return mPlayerDisplayedHealth;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetPlayerDisplayedHealth(const float playerDisplayedHealth)
{
    mPlayerDisplayedHealth = playerDisplayedHealth;
}

///------------------------------------------------------------------------------------------------

long GameSingletons::GetCrystalCount()
{
    return mCrystalCount;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetCrystalCount(const long crystalCount)
{
    mCrystalCount = crystalCount;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetDisplayedCrystalCount()
{
    return mDisplayedCrystalCount;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetDisplayedCrystalCount(const float displayedCrystalCount)
{
    mDisplayedCrystalCount = displayedCrystalCount;
}

///------------------------------------------------------------------------------------------------

MapCoord GameSingletons::GetCurrentMapCoord()
{
    return mCurrentMapCoord;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetCurrentMapCoord(const MapCoord &mapCoord)
{
    mCurrentMapCoord = mapCoord;
}

///------------------------------------------------------------------------------------------------

const std::map<MapCoord, Map::NodeData>& GameSingletons::GetMapData()
{
    return mMapData;
}

///------------------------------------------------------------------------------------------------


void GameSingletons::SetMapData(const std::map<MapCoord, Map::NodeData>& mapData)
{
    mMapData = mapData;
}

///------------------------------------------------------------------------------------------------
