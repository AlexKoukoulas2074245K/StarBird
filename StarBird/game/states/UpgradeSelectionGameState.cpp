///------------------------------------------------------------------------------------------------
///  UpgradeSelectionGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradeSelectionGameState.h"
#include "UpgradesLogicHandler.h"
#include "WaveIntroGameState.h"
#include "../GameObjectConstants.h"
#include "../GameSingletons.h"
#include "../Scene.h"
#include "../SceneObjectUtils.h"
#include "../../utils/MathUtils.h"


///------------------------------------------------------------------------------------------------

const strutils::StringId UpgradeSelectionGameState::STATE_NAME("UpgradeSelectionGameState");

///------------------------------------------------------------------------------------------------

// Preload shine texture to avoid stuttering
UpgradeSelectionGameState::UpgradeSelectionGameState()
    : mShineShaderFileResourceId(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::SHINE_SHADER_FILE_NAME))
    , mShineTextureResourceId(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::UPGRADE_SHINE_EFFECT_TEXTURE_FILE_NAME))
{
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::VInitialize()
{
    mState = SubState::OVERLAY_IN;
    mSelectionState = SelectionState::NONE;
    mAnimationTween = 0.0f;
    CreateUpgradeSceneObjects();
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective UpgradeSelectionGameState::VUpdate(const float dtMillis)
{
    switch (mState)
    {
        case SubState::OVERLAY_IN:
        {
            UpdateOverlayIn(dtMillis);
        } break;
            
        case SubState::UPGRADE_SELECTION:
        {
            UpdateUpgradeSelection(dtMillis);
        } break;
            
        case SubState::SHINE_SELECTION:
        {
            UpdateShineSelection(dtMillis);
        } break;
        
        case SubState::OVERLAY_OUT:
        {
            UpdateOverlayOut(dtMillis);
        } break;
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::VDestroy()
{
    mScene->RemoveAllSceneObjectsWithName(scene_object_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithName(scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithName(scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithName(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithName(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::CreateUpgradeSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mScale = game_object_constants::FULL_SCREEN_OVERLAY_SCALE;
        overlaySo.mPosition = game_object_constants::FULL_SCREEN_OVERLAY_POSITION;
        overlaySo.mName = scene_object_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME;
        overlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mScene->AddSceneObject(std::move(overlaySo));
    }
    
    // Left Upgrade Container
    {
        SceneObject leftUpgradeContainerSO;
        leftUpgradeContainerSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        leftUpgradeContainerSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeContainerSO.mScale = game_object_constants::LEFT_UPGRADE_CONTAINER_SCALE;
        leftUpgradeContainerSO.mPosition = game_object_constants::LEFT_UPGRADE_CONTAINER_INIT_POS;
        leftUpgradeContainerSO.mName = scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(leftUpgradeContainerSO));
    }
    
    // Right Upgrade Container
    {
        SceneObject rightUpgradeContainerSO;
        rightUpgradeContainerSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        rightUpgradeContainerSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeContainerSO.mScale = game_object_constants::RIGHT_UPGRADE_CONTAINER_SCALE;
        rightUpgradeContainerSO.mPosition = game_object_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS;
        rightUpgradeContainerSO.mName = scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(rightUpgradeContainerSO));
    }
    
    // Left Upgrade
    {
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        auto upgradesIter = availableUpgrades.begin();
        std::advance(upgradesIter, math::RandomInt(0, static_cast<int>(availableUpgrades.size() - 1)));
        auto& upgrade = upgradesIter->second;
        
        SceneObject leftUpgradeSO;
        leftUpgradeSO.mAnimation = std::make_unique<SingleFrameAnimation>(upgrade.mTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        leftUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeSO.mScale = game_object_constants::LEFT_UPGRADE_SCALE;
        leftUpgradeSO.mPosition = game_object_constants::LEFT_UPGRADE_INIT_POS;
        leftUpgradeSO.mName = scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(leftUpgradeSO));
        
        GameSingletons::GetUpgradeSelection().first = upgrade;
        availableUpgrades.erase(upgrade.mUpgradeName);
    }
    
    // Right Upgrade
    {
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        auto upgradesIter = availableUpgrades.begin();
        std::advance(upgradesIter, math::RandomInt(0, static_cast<int>(availableUpgrades.size() - 1)));
        auto& upgrade = upgradesIter->second;
        
        SceneObject rightUpgradeSO;
        rightUpgradeSO.mAnimation = std::make_unique<SingleFrameAnimation>(upgrade.mTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        rightUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeSO.mScale = game_object_constants::RIGHT_UPGRADE_SCALE;
        rightUpgradeSO.mPosition = game_object_constants::RIGHT_UPGRADE_INIT_POS;
        rightUpgradeSO.mName = scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(rightUpgradeSO));
        
        GameSingletons::GetUpgradeSelection().second = upgrade;
        availableUpgrades.erase(upgrade.mUpgradeName);
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::UpdateOverlayIn(const float dtMillis)
{
    auto upgradeOverlaySoOpt = mScene->GetSceneObject(scene_object_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME);
    if (upgradeOverlaySoOpt)
    {
        auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
        auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME];
        upgradeOverlayAlpha += dtMillis * game_object_constants::FULL_SCREEN_OVERLAY_DARKENING_SPEED;
        if (upgradeOverlayAlpha >= game_object_constants::FULL_SCREEN_OVERLAY_MAX_ALPHA)
        {
            upgradeOverlayAlpha = game_object_constants::FULL_SCREEN_OVERLAY_MAX_ALPHA;
            mState = SubState::UPGRADE_SELECTION;
        }
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::UpdateUpgradeSelection(const float dtMillis)
{
    mAnimationTween = math::Min(1.0f, mAnimationTween + dtMillis * game_object_constants::UPGRADE_MOVEMENT_SPEED);
    float perc = math::Min(1.0f, math::TweenValue(mAnimationTween, math::BounceFunction, math::TweeningMode::EASE_IN));
    
    auto leftUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto rightUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto leftUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
    auto rightUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
    
    if (leftUpgradeContainerSoOpt)
    {
        leftUpgradeContainerSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (rightUpgradeContainerSoOpt)
    {
        rightUpgradeContainerSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::RIGHT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (leftUpgradeSoOpt)
    {
        leftUpgradeSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_TARGET_POS.x;
    }
    
    if (rightUpgradeSoOpt)
    {
        rightUpgradeSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::RIGHT_UPGRADE_INIT_POS.x + perc * game_object_constants::RIGHT_UPGRADE_TARGET_POS.x;
    }
    
    auto selectedUpgradeName = TestForUpgradeSelected();
    if (selectedUpgradeName != strutils::StringId())
    {
        auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        
        auto& upgradeSelection = GameSingletons::GetUpgradeSelection();
        if (selectedUpgradeName == scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME)
        {
            mSelectionState = SelectionState::LEFT_SELECTED;
            mUpgradesLogicHandler->OnUpgradeEquipped(upgradeSelection.first.mUpgradeName);
            equippedUpgrades[upgradeSelection.first.mUpgradeName] = upgradeSelection.first;
            availableUpgrades[upgradeSelection.second.mUpgradeName] = upgradeSelection.second;
            
            if (leftUpgradeSoOpt)
            {
                auto& leftUpgradeSo = leftUpgradeSoOpt->get();
                leftUpgradeSo.mAnimation = std::make_unique<ShineAnimation>(&leftUpgradeSo, leftUpgradeSo.mAnimation->VGetCurrentTextureResourceId(), mShineTextureResourceId, leftUpgradeSo.mAnimation->VGetCurrentMeshResourceId(), mShineShaderFileResourceId, glm::vec3(1.0f), scene_object_constants::UPGRADE_SHINE_EFFECT_SPEED, false);
            }
        }
        else
        {
            mSelectionState = SelectionState::RIGHT_SELECTED;
            mUpgradesLogicHandler->OnUpgradeEquipped(upgradeSelection.second.mUpgradeName);
            equippedUpgrades[upgradeSelection.second.mUpgradeName] = upgradeSelection.second;
            availableUpgrades[upgradeSelection.first.mUpgradeName] = upgradeSelection.first;
            
            if (rightUpgradeSoOpt)
            {
                auto& rightUpgradeSo = rightUpgradeSoOpt->get();
                rightUpgradeSo.mAnimation = std::make_unique<ShineAnimation>(&rightUpgradeSo, rightUpgradeSo.mAnimation->VGetCurrentTextureResourceId(), mShineTextureResourceId, rightUpgradeSo.mAnimation->VGetCurrentMeshResourceId(), mShineShaderFileResourceId, glm::vec3(1.0f), scene_object_constants::UPGRADE_SHINE_EFFECT_SPEED, false);
            }
        }
        
        mState = SubState::SHINE_SELECTION;
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::UpdateShineSelection(const float dtMillis)
{
    if (mSelectionState == SelectionState::LEFT_SELECTED)
    {
        auto leftUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
        if (leftUpgradeSoOpt)
        {
            auto& leftUgpradeSo = leftUpgradeSoOpt->get();
            
            // Manual animation update since this state returns a BLOCKING update directive
            leftUgpradeSo.mAnimation->VUpdate(dtMillis, leftUgpradeSo);
            
            if (leftUgpradeSo.mShaderFloatUniformValues[scene_object_constants::SHINE_X_OFFSET_UNIFORM_NAME] < scene_object_constants::SHINE_EFFECT_X_OFFSET_END_VAL)
            {
                mState = SubState::OVERLAY_OUT;
            }
        }
    }
    else if (mSelectionState == SelectionState::RIGHT_SELECTED)
    {
        auto rightUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
        if (rightUpgradeSoOpt)
        {
            auto& rightUgpradeSo = rightUpgradeSoOpt->get();
            
            // Manual animation update since this state returns a BLOCKING update directive
            rightUgpradeSo.mAnimation->VUpdate(dtMillis, rightUgpradeSo);
            
            if (rightUgpradeSo.mShaderFloatUniformValues[scene_object_constants::SHINE_X_OFFSET_UNIFORM_NAME] < scene_object_constants::SHINE_EFFECT_X_OFFSET_END_VAL)
            {
                mState = SubState::OVERLAY_OUT;
            }
        }
    }
    else
    {
        assert(false);
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::UpdateOverlayOut(const float dtMillis)
{
    mAnimationTween = math::Max(0.0f, mAnimationTween - dtMillis * game_object_constants::UPGRADE_MOVEMENT_SPEED);
    float perc = math::Max(0.0f, math::TweenValue(mAnimationTween, math::QuadFunction, math::TweeningMode::EASE_OUT));
    
    auto upgradeOverlaySoOpt = mScene->GetSceneObject(scene_object_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME);
    auto leftUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto rightUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto leftUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
    auto rightUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
    auto playerSoOpt = mScene->GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);

    if (leftUpgradeContainerSoOpt)
    {
        leftUpgradeContainerSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (rightUpgradeContainerSoOpt)
    {
        rightUpgradeContainerSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::RIGHT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (leftUpgradeSoOpt)
    {
        leftUpgradeSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_TARGET_POS.x;
    }
    
    if (rightUpgradeSoOpt)
    {
        rightUpgradeSoOpt->get().mPosition.x = (1.0f - perc) * game_object_constants::RIGHT_UPGRADE_INIT_POS.x + perc * game_object_constants::RIGHT_UPGRADE_TARGET_POS.x;
    }
    
    if (upgradeOverlaySoOpt)
    {
        auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
        auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME];
        upgradeOverlayAlpha -= dtMillis * game_object_constants::FULL_SCREEN_OVERLAY_DARKENING_SPEED;
        if (upgradeOverlayAlpha <= 0)
        {
            upgradeOverlayAlpha = 0.0f;
            
            if (playerSoOpt)
            {
                // Override roll effect animation
                playerSoOpt->get().mRotation.y = 0.0f;
                playerSoOpt->get().mAnimation = std::make_unique<ShineAnimation>(&playerSoOpt->get(), playerSoOpt->get().mAnimation->VGetCurrentTextureResourceId(), mShineTextureResourceId, playerSoOpt->get().mAnimation->VGetCurrentMeshResourceId(), mShineShaderFileResourceId,
                    playerSoOpt->get().mAnimation->VGetScale(),
                    scene_object_constants::UPGRADE_SHINE_EFFECT_SPEED, true);
            }
            Complete(WaveIntroGameState::STATE_NAME);
        }
    }
}

///------------------------------------------------------------------------------------------------

strutils::StringId UpgradeSelectionGameState::TestForUpgradeSelected()
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    const auto& inputContext = GameSingletons::GetInputContext();
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto leftUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
        auto rightUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
        
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        // Left Upgrade test
        if (leftUpgradeSoOpt)
        {
            auto& leftUpgrade = leftUpgradeSoOpt->get();
            if (scene_object_utils::IsPointInsideSceneObject(leftUpgrade, touchPos))
            {
                return leftUpgrade.mName;
            }
        }
        
        // Right Upgrade test
        if (rightUpgradeSoOpt)
        {
            auto& rightUpgrade = rightUpgradeSoOpt->get();
            if (scene_object_utils::IsPointInsideSceneObject(rightUpgrade, touchPos))
            {
                return rightUpgrade.mName;
            }
        }
    }
    
    return strutils::StringId();
}

///------------------------------------------------------------------------------------------------
