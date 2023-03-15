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
GameSingletons::UpgradeMap GameSingletons::mEquippedUpgrades = {};
GameSingletons::UpgradeMap GameSingletons::mAvailableUpgrades = {};
std::pair<UpgradeDefinition, UpgradeDefinition> GameSingletons::mUpgradeSelection = {};
float GameSingletons::mGameSpeedMultiplier = 1.0f;
float GameSingletons::mBossMaxHealth = 0.0f;
float GameSingletons::mBossCurrentHealth = 1.0f;

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
