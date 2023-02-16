///------------------------------------------------------------------------------------------------
///  UpgradeSelectionGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradeSelectionGameState.h"
#include "UpgradeOverlayOutGameState.h"
#include "../GameObjectConstants.h"
#include "../GameSingletons.h"
#include "../Scene.h"
#include "../../utils/MathUtils.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId UpgradeSelectionGameState::STATE_NAME("UpgradeSelectionGameState");

///------------------------------------------------------------------------------------------------

void UpgradeSelectionGameState::Initialize()
{
    mAnimationTween = 0.0f;
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective UpgradeSelectionGameState::Update(const float dtMillis)
{
    mAnimationTween = math::Min(1.0f, mAnimationTween + dtMillis * game_object_constants::UPGRADE_MOVEMENT_SPEED);
    float perc = math::Min(1.0f, math::TweenValue(mAnimationTween, math::BounceFunction, math::TweeningMode::EASE_IN));
    
    auto leftUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto rightUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto leftUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
    auto rightUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
    
    if (leftUpgradeContainerSoOpt)
    {
        leftUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (rightUpgradeContainerSoOpt)
    {
        rightUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * game_object_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::RIGHT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (leftUpgradeSoOpt)
    {
        leftUpgradeSoOpt->get().mCustomPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_TARGET_POS.x;
    }
    
    if (rightUpgradeSoOpt)
    {
        rightUpgradeSoOpt->get().mCustomPosition.x = (1.0f - perc) * game_object_constants::RIGHT_UPGRADE_INIT_POS.x + perc * game_object_constants::RIGHT_UPGRADE_TARGET_POS.x;
    }
    
    auto selectedUpgradeName = TestForUpgradeSelected();
    if (selectedUpgradeName != strutils::StringId())
    {
        auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        
        Animation customAnim;
        customAnim.mAnimationType = AnimationType::PULSING;
        customAnim.mAnimDtAccumSpeed = game_object_constants::SELECTED_UPGRADE_PULSE_ANIM_SPEED;
        customAnim.mPulsingEnlargementFactor = game_object_constants::SELECTED_UPGRADE_PULSE_ENLARGEMENT_FACTOR;
        
        auto& upgradeSelection = GameSingletons::GetUpgradeSelection();
        if (selectedUpgradeName == scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME)
        {
            mUpgradesLogicHandler->OnUpgradeEquipped(upgradeSelection.first.mUpgradeName);
            equippedUpgrades[upgradeSelection.first.mUpgradeName] = upgradeSelection.first;
            availableUpgrades[upgradeSelection.second.mUpgradeName] = upgradeSelection.second;
            
            if (leftUpgradeSoOpt)
            {
                auto& leftUpgradeSo = leftUpgradeSoOpt->get();
                leftUpgradeSo.mCustomAnimation = customAnim;
            }
            
            if (leftUpgradeContainerSoOpt)
            {
                auto& leftUpgradeContainerSo = leftUpgradeContainerSoOpt->get();
                leftUpgradeContainerSo.mCustomAnimation = customAnim;
            }
        }
        else
        {
            mUpgradesLogicHandler->OnUpgradeEquipped(upgradeSelection.second.mUpgradeName);
            equippedUpgrades[upgradeSelection.second.mUpgradeName] = upgradeSelection.second;
            availableUpgrades[upgradeSelection.first.mUpgradeName] = upgradeSelection.first;
            
            if (rightUpgradeSoOpt)
            {
                auto& rightUpgradeSo = rightUpgradeSoOpt->get();
                rightUpgradeSo.mCustomAnimation = customAnim;
            }
            
            if (rightUpgradeContainerSoOpt)
            {
                auto& rightUpgradeContainerSo = rightUpgradeContainerSoOpt->get();
                rightUpgradeContainerSo.mCustomAnimation = customAnim;
            }
        }
        
        Complete(UpgradeOverlayOutGameState::STATE_NAME);
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
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
            auto upgradeRectBottomLeft = glm::vec2(leftUpgrade.mCustomPosition.x - leftUpgrade.mCustomScale.x/2, leftUpgrade.mCustomPosition.y - leftUpgrade.mCustomScale.y/2);
            auto upgradeRectTopRight = glm::vec2(leftUpgrade.mCustomPosition.x + leftUpgrade.mCustomScale.x/2, leftUpgrade.mCustomPosition.y + leftUpgrade.mCustomScale.y/2);
            
            if (math::IsPointInsideRectangle(upgradeRectBottomLeft, upgradeRectTopRight, touchPos))
            {
                return leftUpgrade.mNameTag;
            }
        }
        
        // Right Upgrade test
        if (rightUpgradeSoOpt)
        {
            auto& rightUpgrade = rightUpgradeSoOpt->get();
            auto upgradeRectBottomLeft = glm::vec2(rightUpgrade.mCustomPosition.x - rightUpgrade.mCustomScale.x/2, rightUpgrade.mCustomPosition.y - rightUpgrade.mCustomScale.y/2);
            auto upgradeRectTopRight = glm::vec2(rightUpgrade.mCustomPosition.x + rightUpgrade.mCustomScale.x/2, rightUpgrade.mCustomPosition.y + rightUpgrade.mCustomScale.y/2);
            
            if (math::IsPointInsideRectangle(upgradeRectBottomLeft, upgradeRectTopRight, touchPos))
            {
                return rightUpgrade.mNameTag;
            }
        }
    }
    
    return strutils::StringId();
}

///------------------------------------------------------------------------------------------------
