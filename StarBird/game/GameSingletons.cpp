///------------------------------------------------------------------------------------------------
///  GameSingletons.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "GameSingletons.h"
#include "GameConstants.h"

///------------------------------------------------------------------------------------------------

InputContext GameSingletons::mInputContext = {};
SDL_Window* GameSingletons::mWindow = nullptr;
glm::vec2 GameSingletons::mWindowDimensions = glm::vec2();
std::unordered_map<SceneObjectType, Camera> GameSingletons::mSceneObjectTypeToCameraMap = {};
std::vector<UpgradeDefinition> GameSingletons::mEquippedUpgrades = {};
std::vector<UpgradeDefinition> GameSingletons::mAvailableUpgrades = {};
std::vector<UpgradeDefinition> GameSingletons::mEventOnlyUpgrades = {};
MapCoord GameSingletons::mCurrentMapCoord = MapCoord(game_constants::DEFAULT_MAP_COORD_COL, game_constants::DEFAULT_MAP_COORD_ROW);
int GameSingletons::mMapGenerationSeed = 0;
int GameSingletons::mMapLevel = 0;
int GameSingletons::mBackgroundIndex = 0;
int GameSingletons::mResearchCostMultiplier = 1;
long GameSingletons::mCrystalCount = 0;
float GameSingletons::mDisplayedCrystalCount = 0;
float GameSingletons::mGameSpeedMultiplier = 1.0f;
float GameSingletons::mBossMaxHealth = 0.0f;
float GameSingletons::mBossCurrentHealth = 1.0f;
float GameSingletons::mPlayerShieldHealth = 0.0f;
float GameSingletons::mPlayerMaxHealth = 1.0f;
float GameSingletons::mPlayerCurrentHealth = 1.0f;
float GameSingletons::mPlayerDisplayedHealth = 1.0f;
float GameSingletons::mPlayerAttackStat = 0.0f;
float GameSingletons::mPlayerBulletSpeedStat = 0.0f;
float GameSingletons::mPlayerMovementStat = 0.0f;
bool GameSingletons::mGodMode = false;
bool GameSingletons::mErasedLabsOnCurrentMap = false;

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

std::vector<UpgradeDefinition>& GameSingletons::GetEquippedUpgrades()
{
    return mEquippedUpgrades;
}

///------------------------------------------------------------------------------------------------

std::vector<UpgradeDefinition>& GameSingletons::GetAvailableUpgrades()
{
    return mAvailableUpgrades;
}

///------------------------------------------------------------------------------------------------

const std::vector<UpgradeDefinition>& GameSingletons::GetEventOnlyUpgrades()
{
    return mEventOnlyUpgrades;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetEquippedUpgrades(const std::vector<UpgradeDefinition>& upgrades)
{
    mEquippedUpgrades = upgrades;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetAvailableUpgrades(const std::vector<UpgradeDefinition>& upgrades)
{
    mAvailableUpgrades = upgrades;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetEventOnlyUpgrades(const std::vector<UpgradeDefinition>& upgrades)
{
    mEventOnlyUpgrades = upgrades;
}

///------------------------------------------------------------------------------------------------

bool GameSingletons::HasEquippedUpgrade(const strutils::StringId& upgradeNameId)
{
    return std::find_if(mEquippedUpgrades.cbegin(), mEquippedUpgrades.cend(), [&](const UpgradeDefinition& upgradeDefinition){ return upgradeDefinition.mUpgradeNameId == upgradeNameId; }) != mEquippedUpgrades.cend();
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

void GameSingletons::SetPlayerShieldHealth(const float playerShieldHealth)
{
    mPlayerShieldHealth = playerShieldHealth;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetPlayerShieldHealth()
{
    return mPlayerShieldHealth;
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

float GameSingletons::GetPlayerAttackStat()
{
    return mPlayerAttackStat;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetPlayerAttackStat(const float playerAttackStat)
{
    mPlayerAttackStat = playerAttackStat;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetPlayerBulletSpeedStat()
{
    return mPlayerBulletSpeedStat;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetPlayerBulletSpeedStat(const float playerBulletSpeedStat)
{
    mPlayerBulletSpeedStat = playerBulletSpeedStat;
}

///------------------------------------------------------------------------------------------------

float GameSingletons::GetPlayerMovementSpeedStat()
{
    return mPlayerMovementStat;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetPlayerMovementSpeedStat(const float playerMovementStat)
{
    mPlayerMovementStat = playerMovementStat;
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

int GameSingletons::GetMapGenerationSeed()
{
    return mMapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetMapGenerationSeed(const int mapGenerationSeed)
{
    mMapGenerationSeed = mapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

int GameSingletons::GetMapLevel()
{
    return mMapLevel;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetMapLevel(const int mapLevel)
{
    mMapLevel = mapLevel;
}

///------------------------------------------------------------------------------------------------

int GameSingletons::GetBackgroundIndex()
{
    return mBackgroundIndex;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetBackgroundIndex(const int backgroundIndex)
{
    mBackgroundIndex = backgroundIndex;
}

///------------------------------------------------------------------------------------------------

bool GameSingletons::GetGodeMode()
{
    return mGodMode;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetGodMode(const bool godMode)
{
    mGodMode = godMode;
}

///------------------------------------------------------------------------------------------------

bool GameSingletons::GetErasedLabsOnCurrentMap()
{
    return mErasedLabsOnCurrentMap;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetErasedLabsOnCurrentMap(const bool erasedLabsOnCurrentMap)
{
    mErasedLabsOnCurrentMap = erasedLabsOnCurrentMap;
}

///------------------------------------------------------------------------------------------------

int GameSingletons::GetResearchCostMultiplier()
{
    return mResearchCostMultiplier;
}

///------------------------------------------------------------------------------------------------

void GameSingletons::SetResearchCostMultiplier(const int researchCostMultiplier)
{
    mResearchCostMultiplier = researchCostMultiplier;
}

///------------------------------------------------------------------------------------------------
