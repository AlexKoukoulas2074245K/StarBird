///------------------------------------------------------------------------------------------------
///  UpgradeOverlayOutGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradeOverlayOutGameState.h"
#include "WaveIntroGameState.h"
#include "../GameObjectConstants.h"
#include "../LevelUpdater.h"
#include "../Scene.h"
#include "../SceneObjectConstants.h"
#include "../../utils/MathUtils.h"



///------------------------------------------------------------------------------------------------

const strutils::StringId UpgradeOverlayOutGameState::STATE_NAME("UpgradeOverlayOutGameState");

///------------------------------------------------------------------------------------------------

void UpgradeOverlayOutGameState::Initialize()
{
    mAnimationTween = 1.0f;
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective UpgradeOverlayOutGameState::Update(const float dtMillis)
{
    mAnimationTween = math::Max(0.0f, mAnimationTween - dtMillis * game_object_constants::UPGRADE_MOVEMENT_SPEED);
    float perc = math::Max(0.0f, math::TweenValue(mAnimationTween, math::QuadFunction, math::TweeningMode::EASE_OUT));
    
    auto upgradeOverlaySoOpt = mScene->GetSceneObject(scene_object_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
    auto leftUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto rightUpgradeContainerSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    auto leftUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
    auto rightUpgradeSoOpt = mScene->GetSceneObject(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
    
    if (leftUpgradeContainerSoOpt)
    {
        mLevelUpdater->UpdateAnimation(leftUpgradeContainerSoOpt->get(), std::nullopt, dtMillis);
        leftUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * game_object_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * game_object_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
    }
    
    if (rightUpgradeContainerSoOpt)
    {
        mLevelUpdater->UpdateAnimation(rightUpgradeContainerSoOpt->get(), std::nullopt, dtMillis);
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
    
    if (upgradeOverlaySoOpt)
    {
        auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
        auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME];
        upgradeOverlayAlpha -= dtMillis * game_object_constants::OVERLAY_DARKENING_SPEED;
        if (upgradeOverlayAlpha <= 0)
        {
            upgradeOverlayAlpha = 0.0f;
            Complete(WaveIntroGameState::STATE_NAME);
        }
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
}

///------------------------------------------------------------------------------------------------

void UpgradeOverlayOutGameState::Destroy()
{
    mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME);
    mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME);
}

///------------------------------------------------------------------------------------------------
