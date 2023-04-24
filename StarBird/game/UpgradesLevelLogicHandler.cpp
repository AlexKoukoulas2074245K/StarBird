///------------------------------------------------------------------------------------------------
///  UpgradesLevelLogicHandler.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradesLevelLogicHandler.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"
#include "../utils/MathUtils.h"

#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const char* PLAYER_SHIELD_TEXTURE_FILE_NAME = "player_shield_texture_mm.bmp";
static const char* PLAYER_SHIELD_EFFECT_TEXTURE_FILE_NAME = "player_shield_alpha_map_mm.bmp";
static const char* PLAYER_SHIELD_MESH_FILE_NAME = "planet.obj";

static const glm::vec3 LEFT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(-2.0f, -0.5f, 0.0f);
static const glm::vec3 LEFT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 RIGHT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(2.0f, -0.5f, 0.0f);
static const glm::vec3 RIGHT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 PLAYER_SHIELD_POSITION_OFFSET = glm::vec3(0.0f, 0.0f, 0.5f);
static const glm::vec3 PLAYER_SHIELD_SCALE = glm::vec3(1.5f, 1.5f, 1.5f);

static const float PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR = 1.0f/200.0f;
static const float PLAYER_PULSE_SHIELD_ANIM_SPEED = 0.01f;
static const float PLAYER_SHIELD_ROTATION_SPEED = 0.001f;

///------------------------------------------------------------------------------------------------

UpgradesLevelLogicHandler::UpgradesLevelLogicHandler(Scene& scene)
    : mScene(scene)
{
}

///------------------------------------------------------------------------------------------------

void UpgradesLevelLogicHandler::InitializeEquippedUpgrade(const strutils::StringId& upgradeId)
{
    if (upgradeId == game_constants::MIRROR_IMAGE_UGPRADE_NAME)
    {
        CreateMirrorImageSceneObjects();
    }
    else if (upgradeId == game_constants::PLAYER_SHIELD_UPGRADE_NAME)
    {
        CreatePlayerShieldSceneObject();
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLevelLogicHandler::Update(const float dtMillis)
{
    bool mirrorImageEquipped = GameSingletons::HasEquippedUpgrade(game_constants::MIRROR_IMAGE_UGPRADE_NAME);
    bool shieldEquipped = GameSingletons::HasEquippedUpgrade(game_constants::PLAYER_SHIELD_UPGRADE_NAME);
    
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

void UpgradesLevelLogicHandler::CreateMirrorImageSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    {
        SceneObject leftMirrorImageSo;
        leftMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        leftMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        leftMirrorImageSo.mPosition = LEFT_MIRROR_IMAGE_POSITION_OFFSET;
        leftMirrorImageSo.mScale = LEFT_MIRROR_IMAGE_SCALE;
        leftMirrorImageSo.mName = game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
        leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
        mScene.AddSceneObject(std::move(leftMirrorImageSo));
    }
    
    {
        SceneObject rightMirrorImageSo;
        rightMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        rightMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        rightMirrorImageSo.mPosition = RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
        rightMirrorImageSo.mScale = RIGHT_MIRROR_IMAGE_SCALE;
        rightMirrorImageSo.mName = game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
        rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
        mScene.AddSceneObject(std::move(rightMirrorImageSo));
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLevelLogicHandler::CreatePlayerShieldSceneObject()
{
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (playerSoOpt)
    {
        auto& resService = resources::ResourceLoadingService::GetInstance();
        SceneObject playerShieldSo;
        
        playerShieldSo.mAnimation = std::make_unique<PlayerShieldAnimation>(&playerShieldSo, resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PLAYER_SHIELD_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PLAYER_SHIELD_EFFECT_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + PLAYER_SHIELD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::PLAYER_SHIELD_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        playerShieldSo.mAnimation->SetCompletionCallback([&]()
        {
            mScene.RemoveAllSceneObjectsWithName(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
        });
        
        playerShieldSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        playerShieldSo.mPosition = math::Box2dVec2ToGlmVec3(playerSoOpt->get().mBody->GetWorldCenter()) + PLAYER_SHIELD_POSITION_OFFSET;
        playerShieldSo.mScale = PLAYER_SHIELD_SCALE;
        playerShieldSo.mName = game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME;
        
        playerShieldSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(playerShieldSo.mAnimation->VGetCurrentTextureResourceId(), playerShieldSo.mAnimation->VGetCurrentMeshResourceId(), playerShieldSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, PLAYER_SHIELD_ROTATION_SPEED, false));
        
        playerShieldSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(playerShieldSo.mAnimation->VGetCurrentTextureResourceId(), playerShieldSo.mAnimation->VGetCurrentMeshResourceId(), playerShieldSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.0f), PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, PLAYER_PULSE_SHIELD_ANIM_SPEED, PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR, false));
        
        mScene.AddSceneObject(std::move(playerShieldSo));
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLevelLogicHandler::UpdateMirrorImages(const float)
{
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto leftMirrorImageSoOpt = mScene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    auto rightMirrorImageSoOpt = mScene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    
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
        
        leftMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + LEFT_MIRROR_IMAGE_POSITION_OFFSET;
        rightMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLevelLogicHandler::UpdatePlayerShield(const float dtMillis)
{
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerShieldSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME);
    
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
        playerShieldSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + PLAYER_SHIELD_POSITION_OFFSET;
    }
}

///------------------------------------------------------------------------------------------------
