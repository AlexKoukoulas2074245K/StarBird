///------------------------------------------------------------------------------------------------
///  UpgradeOverlayInGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradeOverlayInGameState.h"
#include "UpgradeSelectionGameState.h"
#include "../GameObjectConstants.h"
#include "../GameSingletons.h"
#include "../Scene.h"
#include "../SceneObjectConstants.h"
#include "../SceneObject.h"
#include "../../resloading/ResourceLoadingService.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId UpgradeOverlayInGameState::STATE_NAME("UpgradeOverlayInGameState");

///------------------------------------------------------------------------------------------------

void UpgradeOverlayInGameState::Initialize()
{
    CreateUpgradeSceneObjects();
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective UpgradeOverlayInGameState::Update(const float dtMillis)
{
    auto upgradeOverlaySoOpt = mScene->GetSceneObject(scene_object_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
    if (upgradeOverlaySoOpt)
    {
        auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
        auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME];
        upgradeOverlayAlpha += dtMillis * game_object_constants::OVERLAY_DARKENING_SPEED;
        if (upgradeOverlayAlpha >= game_object_constants::UPGRADE_OVERLAY_MAX_ALPHA)
        {
            upgradeOverlayAlpha = game_object_constants::UPGRADE_OVERLAY_MAX_ALPHA;
            Complete(UpgradeSelectionGameState::STATE_NAME);
        }
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
}

///------------------------------------------------------------------------------------------------

void UpgradeOverlayInGameState::CreateUpgradeSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
        overlaySo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::UPGRADE_OVERLAY_TEXTURE_FILE_NAME);
        overlaySo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mCustomScale = game_object_constants::UPGRADE_OVERLAY_SCALE;
        overlaySo.mCustomPosition = game_object_constants::UPGRADE_OVERLAY_POSITION;
        overlaySo.mNameTag = scene_object_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME;
        overlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mScene->AddSceneObject(std::move(overlaySo));
    }
    
    // Left Upgrade Container
    {
        SceneObject leftUpgradeContainerSO;
        leftUpgradeContainerSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        leftUpgradeContainerSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME);
        leftUpgradeContainerSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        leftUpgradeContainerSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeContainerSO.mCustomScale = game_object_constants::LEFT_UPGRADE_CONTAINER_SCALE;
        leftUpgradeContainerSO.mCustomPosition = game_object_constants::LEFT_UPGRADE_CONTAINER_INIT_POS;
        leftUpgradeContainerSO.mNameTag = scene_object_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(leftUpgradeContainerSO));
    }
    
    // Right Upgrade Container
    {
        SceneObject rightUpgradeContainerSO;
        rightUpgradeContainerSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        rightUpgradeContainerSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME);
        rightUpgradeContainerSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        rightUpgradeContainerSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeContainerSO.mCustomScale = game_object_constants::RIGHT_UPGRADE_CONTAINER_SCALE;
        rightUpgradeContainerSO.mCustomPosition = game_object_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS;
        rightUpgradeContainerSO.mNameTag = scene_object_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(rightUpgradeContainerSO));
    }
    
    // Left Upgrade
    {
        auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
        auto upgradesIter = availableUpgrades.begin();
        std::advance(upgradesIter, math::RandomInt(0, static_cast<int>(availableUpgrades.size() - 1)));
        auto& upgrade = upgradesIter->second;
        
        SceneObject leftUpgradeSO;
        leftUpgradeSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        leftUpgradeSO.mTextureResourceId = upgrade.mTextureResourceId;
        leftUpgradeSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        leftUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeSO.mCustomScale = game_object_constants::LEFT_UPGRADE_SCALE;
        leftUpgradeSO.mCustomPosition = game_object_constants::LEFT_UPGRADE_INIT_POS;
        leftUpgradeSO.mNameTag = scene_object_constants::LEFT_UPGRADE_SCENE_OBJECT_NAME;
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
        rightUpgradeSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        rightUpgradeSO.mTextureResourceId = upgrade.mTextureResourceId;
        rightUpgradeSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        rightUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeSO.mCustomScale = game_object_constants::RIGHT_UPGRADE_SCALE;
        rightUpgradeSO.mCustomPosition = game_object_constants::RIGHT_UPGRADE_INIT_POS;
        rightUpgradeSO.mNameTag = scene_object_constants::RIGHT_UPGRADE_SCENE_OBJECT_NAME;
        mScene->AddSceneObject(std::move(rightUpgradeSO));
        
        GameSingletons::GetUpgradeSelection().second = upgrade;
        availableUpgrades.erase(upgrade.mUpgradeName);
    }
}
