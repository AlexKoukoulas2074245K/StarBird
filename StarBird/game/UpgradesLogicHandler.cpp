///------------------------------------------------------------------------------------------------
///  UpgradesLogicHandler.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "LevelUpdater.h"
#include "UpgradesLogicHandler.h"
#include "GameObjectConstants.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "SceneObjectConstants.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

UpgradesLogicHandler::UpgradesLogicHandler(Scene& scene, LevelUpdater& levelUpdater)
    : mScene(scene)
    , mLevelUpdater(levelUpdater)
{
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::OnUpgradeEquipped(const strutils::StringId& upgradeId)
{
    if (upgradeId == game_object_constants::MIRROR_IMAGE_UGPRADE_NAME)
    {
        CreateMirrorImageSceneObjects();
    }
    else if (upgradeId == game_object_constants::BULLET_SPEED_UGPRADE_NAME)
    {
        auto bulletFlowOpt = mLevelUpdater.GetFlow(game_object_constants::PLAYER_BULLET_FLOW_NAME);
        if (bulletFlowOpt)
        {
            bulletFlowOpt->get().SetDuration(game_object_constants::HASTENED_PLAYER_BULLET_FLOW_DELAY_MILLIS);
        }
    }
    else if (upgradeId == game_object_constants::PLAYER_SPEED_UGPRADE_NAME)
    {
        auto playerTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetMutableObjectTypeDefinition(game_object_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        if (playerTypeDefOpt)
        {
            playerTypeDefOpt->get().mSpeed *= game_object_constants::PLAYER_SPEED_UPGRADE_MULTIPLIER;
        }
    }
    else if (upgradeId == game_object_constants::PLAYER_SHIELD_UPGRADE_NAME)
    {
        CreatePlayerShieldSceneObject();
    }
    else if (upgradeId == game_object_constants::PLAYER_HEALTH_POTION_UGPRADE_NAME)
    {
        auto playerTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetMutableObjectTypeDefinition(game_object_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        auto playerSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
        
        if (playerSoOpt && playerTypeDefOpt)
        {
            playerSoOpt->get().mHealth = math::Min(playerTypeDefOpt->get().mHealth, playerSoOpt->get().mHealth + game_object_constants::HEALTH_POTION_HEALTH_GAIN);
        }
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::OnUpdate(const float dtMillis)
{
    auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
    bool mirrorImageEquipped = equippedUpgrades.count(game_object_constants::MIRROR_IMAGE_UGPRADE_NAME) != 0;
    bool shieldEquipped = equippedUpgrades.count(game_object_constants::PLAYER_SHIELD_UPGRADE_NAME) != 0;
    
    if (mirrorImageEquipped)
    {
        UpdateMirrorImages(dtMillis);
    }
    if (shieldEquipped)
    {
        UpdatePlayerShield(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::CreateMirrorImageSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    {
        SceneObject leftMirrorImageSo;
        leftMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        leftMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        leftMirrorImageSo.mPosition = game_object_constants::LEFT_MIRROR_IMAGE_POSITION_OFFSET;
        leftMirrorImageSo.mScale = game_object_constants::LEFT_MIRROR_IMAGE_SCALE;
        leftMirrorImageSo.mName = scene_object_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
        leftMirrorImageSo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
        mScene.AddSceneObject(std::move(leftMirrorImageSo));
    }
    
    {
        SceneObject rightMirrorImageSo;
        rightMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        rightMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        rightMirrorImageSo.mPosition = game_object_constants::RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
        rightMirrorImageSo.mScale = game_object_constants::RIGHT_MIRROR_IMAGE_SCALE;
        rightMirrorImageSo.mName = scene_object_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
        rightMirrorImageSo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
        mScene.AddSceneObject(std::move(rightMirrorImageSo));
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::CreatePlayerShieldSceneObject()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    auto playerSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (playerSoOpt)
    {
        SceneObject playerShieldSo;
        playerShieldSo.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::PLAYER_SHIELD_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f),  game_object_constants::PLAYER_PULSE_SHIELD_ANIM_SPEED, game_object_constants::PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR, false);
        playerShieldSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        playerShieldSo.mPosition = math::Box2dVec2ToGlmVec3(playerSoOpt->get().mBody->GetWorldCenter()) + game_object_constants::PLAYER_SHIELD_POSITION_OFFSET;
        playerShieldSo.mScale = game_object_constants::PLAYER_SHIELD_SCALE;
        playerShieldSo.mName = scene_object_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(playerShieldSo));
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::UpdateMirrorImages(const float)
{
    auto playerSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    auto leftMirrorImageSoOpt = mScene.GetSceneObject(scene_object_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    auto rightMirrorImageSoOpt = mScene.GetSceneObject(scene_object_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    
    if (!playerSoOpt)
    {
        if (leftMirrorImageSoOpt)
        {
            leftMirrorImageSoOpt->get().mInvisible = true;
        }
        if (rightMirrorImageSoOpt)
        {
            rightMirrorImageSoOpt->get().mInvisible = true;
        }
    }
    else if (leftMirrorImageSoOpt && rightMirrorImageSoOpt)
    {
        auto& playerSo = playerSoOpt->get();
        auto& leftMirrorImageSo = leftMirrorImageSoOpt->get();
        auto& rightMirrorImageSo = rightMirrorImageSoOpt->get();
        
        leftMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + game_object_constants::LEFT_MIRROR_IMAGE_POSITION_OFFSET;
        rightMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + game_object_constants::RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::UpdatePlayerShield(const float dtMillis)
{
    auto playerSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerShieldSoOpt = mScene.GetSceneObject(scene_object_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
    
    if (!playerSoOpt)
    {
        if (playerShieldSoOpt)
        {
            playerShieldSoOpt->get().mInvisible = true;
        }
    }
    else if (playerShieldSoOpt)
    {
        auto& playerSo = playerSoOpt->get();
        auto& playerShieldSo = playerShieldSoOpt->get();
        playerShieldSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + game_object_constants::PLAYER_SHIELD_POSITION_OFFSET;
    }
}

///------------------------------------------------------------------------------------------------
