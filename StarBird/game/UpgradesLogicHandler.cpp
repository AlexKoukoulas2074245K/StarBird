///------------------------------------------------------------------------------------------------
///  UpgradesLogicHandler.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradesLogicHandler.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"
#include "../utils/MathUtils.h"

#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const char* PLAYER_SHIELD_TEXTURE_FILE_NAME = "player_shield.bmp";
static const std::string DROPPED_CRYSTAL_NAME_PREFIX = "DROPPED_CRYSTAL_";

static const glm::vec3 LEFT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(-2.0f, -0.5f, 0.0f);
static const glm::vec3 LEFT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 RIGHT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(2.0f, -0.5f, 0.0f);
static const glm::vec3 RIGHT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 PLAYER_SHIELD_POSITION_OFFSET = glm::vec3(0.0f, 0.5f, 0.5f);
static const glm::vec3 PLAYER_SHIELD_SCALE = glm::vec3(4.0f, 4.0f, 1.0f);
static const glm::vec3 DROPPED_CRYSTALS_POSITION = glm::vec3(0.0f, 5.0f, 3.0f);

static const float DROPPED_CRYSTAL_SPEED = 0.0009f;
static const float DROPPED_CRYSTAL_DISTANCE_FACTOR = 24.0f;
static const float DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG = 0.5f;
static const float DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG = 2.0f;
static const float COLLECTED_CRYSTAL_PULSING_SPEED = 0.02f;
static const float COLLECTED_CRYSTAL_PULSING_FACTOR = 0.01f;

static const float HEALTH_POTION_HEALTH_GAIN = 100.0f;
static const float PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR = 1.0f/50.0f;
static const float PLAYER_PULSE_SHIELD_ANIM_SPEED = 0.01f;

static const int CRYSTALS_REWARD_COUNT = 50;

///------------------------------------------------------------------------------------------------

UpgradesLogicHandler::UpgradesLogicHandler(Scene& scene)
    : mScene(scene)
{
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::InitializeEquippedUpgrade(const strutils::StringId& upgradeId)
{
    if (upgradeId == game_constants::MIRROR_IMAGE_UGPRADE_NAME)
    {
        CreateMirrorImageSceneObjects();
    }
    else if (upgradeId == game_constants::PLAYER_SHIELD_UPGRADE_NAME)
    {
        CreatePlayerShieldSceneObject();
    }
    else if (upgradeId == game_constants::PLAYER_HEALTH_POTION_UGPRADE_NAME)
    {
        auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        if (playerSoOpt)
        {
            GameSingletons::SetPlayerCurrentHealth(math::Min(GameSingletons::GetPlayerMaxHealth(), GameSingletons::GetPlayerCurrentHealth() + HEALTH_POTION_HEALTH_GAIN));
        }
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::AnimateUpgradeGained(const strutils::StringId& upgradeId)
{
    if (upgradeId == game_constants::CRYSTALS_GIFT_UGPRADE_NAME)
    {
        AnimateCrystalGiftUpgradeGained();
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::Update(const float dtMillis)
{
    auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
    bool mirrorImageEquipped = equippedUpgrades.count(game_constants::MIRROR_IMAGE_UGPRADE_NAME) != 0;
    bool shieldEquipped = equippedUpgrades.count(game_constants::PLAYER_SHIELD_UPGRADE_NAME) != 0;
    
    if (mirrorImageEquipped)
    {
        UpdateMirrorImages(dtMillis);
    }
    if (shieldEquipped)
    {
        UpdatePlayerShield(dtMillis);
    }
    
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::CreateMirrorImageSceneObjects()
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

void UpgradesLogicHandler::CreatePlayerShieldSceneObject()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (playerSoOpt)
    {
        SceneObject playerShieldSo;
        playerShieldSo.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PLAYER_SHIELD_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, PLAYER_PULSE_SHIELD_ANIM_SPEED, PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR, false);
        playerShieldSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        playerShieldSo.mPosition = math::Box2dVec2ToGlmVec3(playerSoOpt->get().mBody->GetWorldCenter()) + PLAYER_SHIELD_POSITION_OFFSET;
        playerShieldSo.mScale = PLAYER_SHIELD_SCALE;
        playerShieldSo.mName = game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(playerShieldSo));
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::AnimateCrystalGiftUpgradeGained()
{
    for (int i = 0; i < CRYSTALS_REWARD_COUNT; ++i)
    {
        mFlows.emplace_back([this]()
        {
            auto& resService = resources::ResourceLoadingService::GetInstance();
            
            SceneObject crystalSo;
            
            glm::vec3 firstControlPoint(DROPPED_CRYSTALS_POSITION + glm::vec3(math::RandomFloat(-DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG), math::RandomFloat(-DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG), 0.0f));
            glm::vec3 thirdControlPoint(game_constants::GUI_CRYSTAL_POSITION);
            glm::vec3 secondControlPoint((thirdControlPoint + firstControlPoint) * 0.5f + glm::vec3(math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), 0.0f));
            
            firstControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
            secondControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
            thirdControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
            
            float speedNoise = math::RandomFloat(-DROPPED_CRYSTAL_SPEED/5, DROPPED_CRYSTAL_SPEED/5);
            float speedMultiplier = DROPPED_CRYSTAL_DISTANCE_FACTOR/glm::distance(firstControlPoint, game_constants::GUI_CRYSTAL_POSITION);
            
            const strutils::StringId droppedCrystalName = strutils::StringId(DROPPED_CRYSTAL_NAME_PREFIX + std::to_string(SDL_GetPerformanceCounter()));
            
            crystalSo.mAnimation = std::make_unique<BezierCurvePathAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), math::BezierCurve({firstControlPoint, secondControlPoint, thirdControlPoint}), (DROPPED_CRYSTAL_SPEED + speedNoise) * speedMultiplier, false);
            
            crystalSo.mAnimation->SetCompletionCallback([droppedCrystalName, this]()
            {
                auto crystalHolderSoOpt = mScene.GetSceneObject(game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME);
                if (crystalHolderSoOpt)
                {
                    auto& crystalHolderSo = crystalHolderSoOpt->get();
                    crystalHolderSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
                    
                    crystalHolderSo.mExtraCompoundingAnimations.clear();
                    crystalHolderSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(crystalHolderSo.mAnimation->VGetCurrentTextureResourceId(), crystalHolderSo.mAnimation->VGetCurrentMeshResourceId(), crystalHolderSo.mAnimation->VGetCurrentShaderResourceId(), game_constants::GUI_CRYSTAL_SCALE, PulsingAnimation::PulsingMode::OUTER_PULSE_ONCE, 0.0f, COLLECTED_CRYSTAL_PULSING_SPEED, COLLECTED_CRYSTAL_PULSING_FACTOR, false));
                }
                
                mScene.RemoveAllSceneObjectsWithName(droppedCrystalName);
                GameSingletons::SetCrystalCount(GameSingletons::GetCrystalCount() + 1);
            });
            
            crystalSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false));
            
            crystalSo.mSceneObjectType = SceneObjectType::GUIObject;
            crystalSo.mPosition = firstControlPoint;
            crystalSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
            crystalSo.mName = droppedCrystalName;
            mScene.AddSceneObject(std::move(crystalSo));
        }, i * game_constants::DROPPED_CRYSTALS_CREATION_STAGGER_MILLIS, RepeatableFlow::RepeatPolicy::ONCE);
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::UpdateMirrorImages(const float)
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

void UpgradesLogicHandler::UpdatePlayerShield(const float dtMillis)
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
