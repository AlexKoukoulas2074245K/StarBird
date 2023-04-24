///------------------------------------------------------------------------------------------------
///  UpgradeUnlockedHandler.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "BlueprintFlows.h"
#include "UpgradeUnlockedHandler.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "PhysicsCollisionListener.h"
#include "PhysicsConstants.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"
#include "../utils/MathUtils.h"

#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const char* PLAYER_SHIELD_TEXTURE_FILE_NAME = "player_shield_texture_mm.bmp";
static const char* PLAYER_SHIELD_EFFECT_TEXTURE_FILE_NAME = "player_shield_alpha_map_mm.bmp";
static const char* PLAYER_SHIELD_MESH_FILE_NAME = "planet.obj";

static const std::string DROPPED_CRYSTAL_NAME_PREFIX = "DROPPED_CRYSTAL_";
static const strutils::StringId HEALTH_UP_ANIMATION_SO_NAME = strutils::StringId("HEALTH_UP_ANIMATION");

static const strutils::StringId DOUBLE_BULLET_FLOW_CREATION_NAME = strutils::StringId("DOUBLE_BULLET_FLOW_CREATION");
static const strutils::StringId MIRROR_IMAGE_FLOW_CREATION_NAME = strutils::StringId("MIRROR_IMAGE_FLOW_CREATION");
static const strutils::StringId ANIMATION_END_FLOW_NAME = strutils::StringId("ANIMATION_END");

static const glm::vec3 LEFT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(-2.0f, -0.5f, 0.0f);
static const glm::vec3 LEFT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 RIGHT_MIRROR_IMAGE_POSITION_OFFSET = glm::vec3(2.0f, -0.5f, 0.0f);
static const glm::vec3 RIGHT_MIRROR_IMAGE_SCALE = glm::vec3(1.5f, 1.5f, 1.0f);

static const glm::vec3 PLAYER_SHIELD_POSITION_OFFSET = glm::vec3(0.0f, 0.0f, 0.5f);
static const glm::vec3 PLAYER_SHIELD_SCALE = glm::vec3(1.5f, 1.5f, 1.5f);
static const glm::vec3 DROPPED_CRYSTALS_POSITION = glm::vec3(0.0f, 5.0f, 3.0f);

static const float DROPPED_CRYSTAL_SPEED = 0.0009f;
static const float DROPPED_CRYSTAL_DISTANCE_FACTOR = 24.0f;
static const float DROPPED_CRYSTAL_FIRST_CONTROL_POINT_NOISE_MAG = 0.5f;
static const float DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG = 2.0f;
static const float COLLECTED_CRYSTAL_PULSING_SPEED = 0.02f;
static const float COLLECTED_CRYSTAL_PULSING_FACTOR = 0.01f;

static const float PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR = 1.0f/200.0f;
static const float PLAYER_PULSE_SHIELD_ANIM_SPEED = 0.01f;
static const float SCENE_OBJECT_FADE_IN_ALPHA_SPEED = 0.001f;
static const float INTER_ANIMATION_DELAY_MILLIS = 3000.0f;
static const float PLAYER_SHIELD_ROTATION_SPEED = 0.001f;

static const int CRYSTALS_REWARD_COUNT = 50;

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeUnlockedHandler(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mForceFinishAnimation(false)
{
    // BULLET_TOP_WALL
    {
        const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
        assert(worldCamOpt);
        const auto& worldCam = worldCamOpt->get();
        
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, -2.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(worldCam.GetCameraLenseWidth()/2.0, 0.1f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = true;
        so.mPosition.z = game_constants::WALL_Z;
        so.mName = game_constants::WALL_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(so));
    }
    
    static PhysicsCollisionListener collisionListener;
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_BULLET_CATEGORY_BIT, physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        const auto& playerBulletName = *static_cast<strutils::StringId*>(firstBody->GetUserData());
        mScene.RemoveAllSceneObjectsWithName(playerBulletName);
    });
    
    mBox2dWorld.SetContactListener(&collisionListener);
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedHandler::OnUpgradeGained(const strutils::StringId& upgradeNameId)
{
    mCurrentUpgradeNameUnlocked = upgradeNameId;
    
    auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
    auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
    
    const auto availableUpgradeIter = std::find_if(availableUpgrades.begin(), availableUpgrades.end(), [&](const UpgradeDefinition& upgradeDefinition){ return upgradeDefinition.mUpgradeNameId == upgradeNameId; });
    
    assert(availableUpgradeIter != availableUpgrades.cend());
    const auto& upgradeDefinition = *availableUpgradeIter;
    
    if (upgradeDefinition.mEquippable)
    {
        if (!equippedUpgrades.empty())
        {
            for (int i = 0; i < equippedUpgrades.size(); ++i)
            {
                if (equippedUpgrades[i].mUpgradeNameId == upgradeNameId)
                {
                    equippedUpgrades.erase(equippedUpgrades.begin() + i);
                    break;
                }
            }
        }
        
        equippedUpgrades.push_back(upgradeDefinition);
    }
    
    if (upgradeDefinition.mIntransient == false)
    {
        availableUpgrades.erase(availableUpgradeIter);
    }
    
    if (mCurrentUpgradeNameUnlocked == game_constants::CRYSTALS_GIFT_UGPRADE_NAME)
    {
        OnCrystalGiftUpgradeGained();
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::PLAYER_HEALTH_POTION_UGPRADE_NAME)
    {
        OnHealthPotionUpgradeGained();
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::MIRROR_IMAGE_UGPRADE_NAME)
    {
        OnMirrorImageUpgradeGained();
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::DOUBLE_BULLET_UGPRADE_NAME)
    {
        OnDoubleBulletUpgradeGained();
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::PLAYER_SHIELD_UPGRADE_NAME)
    {
        OnPlayerShieldUpgradeGained();
    }
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeAnimationState UpgradeUnlockedHandler::Update(const float dtMillis)
{
    mBox2dWorld.Step(physics_constants::WORLD_STEP * GameSingletons::GetGameSpeedMultiplier(), physics_constants::WORLD_VELOCITY_ITERATIONS, physics_constants::WORLD_POSITION_ITERATIONS);
    
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
    
    if (mForceFinishAnimation)
    {
        return UpgradeAnimationState::FINISHED;
    }
    
    if (mCurrentUpgradeNameUnlocked == game_constants::CRYSTALS_GIFT_UGPRADE_NAME)
    {
        return UpdateCrystalGiftUpgradeGained(dtMillis);
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::PLAYER_HEALTH_POTION_UGPRADE_NAME)
    {
        return UpdateHealthPotionUpgradeGained(dtMillis);
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::MIRROR_IMAGE_UGPRADE_NAME)
    {
        return UpdateMirrorImageUpgradeGained(dtMillis);
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::DOUBLE_BULLET_UGPRADE_NAME)
    {
        return UpdateDoubleBulletUpgradeGained(dtMillis);
    }
    else if (mCurrentUpgradeNameUnlocked == game_constants::PLAYER_SHIELD_UPGRADE_NAME)
    {
        return UpdatePlayerShieldUpgradeGained(dtMillis);
    }
    else
    {
        return UpgradeAnimationState::FINISHED;
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedHandler::OnCrystalGiftUpgradeGained()
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
                
                mCreatedSceneObjectNames.erase(std::find(mCreatedSceneObjectNames.begin(), mCreatedSceneObjectNames.end(), droppedCrystalName));
                mScene.RemoveAllSceneObjectsWithName(droppedCrystalName);
                GameSingletons::SetCrystalCount(GameSingletons::GetCrystalCount() + 1);
            });
            
            crystalSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false));
            
            crystalSo.mSceneObjectType = SceneObjectType::GUIObject;
            crystalSo.mPosition = firstControlPoint;
            crystalSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
            crystalSo.mName = droppedCrystalName;
            mCreatedSceneObjectNames.push_back(crystalSo.mName);
            mScene.AddSceneObject(std::move(crystalSo));
        }, i * game_constants::DROPPED_CRYSTALS_CREATION_STAGGER_MILLIS, RepeatableFlow::RepeatPolicy::ONCE);
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedHandler::OnHealthPotionUpgradeGained()
{
    SceneObject healthUpAnimationSo;
    healthUpAnimationSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
    healthUpAnimationSo.mName = HEALTH_UP_ANIMATION_SO_NAME;
    healthUpAnimationSo.mScale = glm::vec3(1.0f);
    healthUpAnimationSo.mSceneObjectType = SceneObjectType::GUIObject;
    healthUpAnimationSo.mInvisible = true;
    healthUpAnimationSo.mAnimation = std::make_unique<HealthUpParticlesAnimation>(mScene, game_constants::PLAYER_HEALTH_BAR_POSITION);
    mScene.AddSceneObject(std::move(healthUpAnimationSo));
    
    GameSingletons::SetPlayerCurrentHealth(GameSingletons::GetPlayerMaxHealth());
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedHandler::OnMirrorImageUpgradeGained()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Vessel
    {
        auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::MIRROR_IMAGE_BULLET_TYPE);
        
        auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject playerSO = scene_object_utils::CreateSceneObjectWithBody(playerObjectDef, game_constants::PLAYER_CHEST_REWARD_POS, mBox2dWorld, game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        playerSO.mAnimation = std::make_unique<SingleFrameAnimation>(playerSO.mAnimation->VGetCurrentTextureResourceId(), playerSO.mAnimation->VGetCurrentMeshResourceId(), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), true);
        playerSO.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        playerSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        
        mScene.AddSceneObject(std::move(playerSO));
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedHandler::OnDoubleBulletUpgradeGained()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Vessel
    {
        auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::MIRROR_IMAGE_BULLET_TYPE);
        
        auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject playerSO = scene_object_utils::CreateSceneObjectWithBody(playerObjectDef, game_constants::PLAYER_CHEST_REWARD_POS, mBox2dWorld, game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        playerSO.mAnimation = std::make_unique<SingleFrameAnimation>(playerSO.mAnimation->VGetCurrentTextureResourceId(), playerSO.mAnimation->VGetCurrentMeshResourceId(), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), true);
        playerSO.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        playerSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        
        mScene.AddSceneObject(std::move(playerSO));
    }
    
    auto playerOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    if (playerOpt && GameSingletons::HasEquippedUpgrade(game_constants::MIRROR_IMAGE_UGPRADE_NAME))
    {
        auto& playerSo = playerOpt->get();
        {
            SceneObject leftMirrorImageSo;
            leftMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            leftMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
            leftMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + LEFT_MIRROR_IMAGE_POSITION_OFFSET;
            leftMirrorImageSo.mScale = LEFT_MIRROR_IMAGE_SCALE;
            leftMirrorImageSo.mName = game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
            leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            mScene.AddSceneObject(std::move(leftMirrorImageSo));
        }
        
        {
            SceneObject rightMirrorImageSo;
            rightMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            rightMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
            rightMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
            rightMirrorImageSo.mScale = RIGHT_MIRROR_IMAGE_SCALE;
            rightMirrorImageSo.mName = game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
            rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            mScene.AddSceneObject(std::move(rightMirrorImageSo));
        }
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedHandler::OnPlayerShieldUpgradeGained()
{
    GameSingletons::SetPlayerShieldHealth(GameSingletons::GetPlayerMaxHealth() * 0.2f);
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Vessel
    {
        auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_constants::MIRROR_IMAGE_BULLET_TYPE);
        
        auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject playerSO = scene_object_utils::CreateSceneObjectWithBody(playerObjectDef, game_constants::PLAYER_CHEST_REWARD_POS, mBox2dWorld, game_constants::PLAYER_SCENE_OBJECT_NAME);
        
        playerSO.mAnimation = std::make_unique<SingleFrameAnimation>(playerSO.mAnimation->VGetCurrentTextureResourceId(), playerSO.mAnimation->VGetCurrentMeshResourceId(), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), true);
        playerSO.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        playerSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        
        mScene.AddSceneObject(std::move(playerSO));
    }
    
    auto playerOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    if (playerOpt && GameSingletons::HasEquippedUpgrade(game_constants::MIRROR_IMAGE_UGPRADE_NAME))
    {
        auto& playerSo = playerOpt->get();
        {
            SceneObject leftMirrorImageSo;
            leftMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            leftMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
            leftMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + LEFT_MIRROR_IMAGE_POSITION_OFFSET;
            leftMirrorImageSo.mScale = LEFT_MIRROR_IMAGE_SCALE;
            leftMirrorImageSo.mName = game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
            leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            mScene.AddSceneObject(std::move(leftMirrorImageSo));
        }
        
        {
            SceneObject rightMirrorImageSo;
            rightMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            rightMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
            rightMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
            rightMirrorImageSo.mScale = RIGHT_MIRROR_IMAGE_SCALE;
            rightMirrorImageSo.mName = game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
            rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            mScene.AddSceneObject(std::move(rightMirrorImageSo));
        }
    }
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeAnimationState UpgradeUnlockedHandler::UpdateCrystalGiftUpgradeGained(const float)
{
    return mCreatedSceneObjectNames.size() == 0 ? UpgradeAnimationState::FINISHED : UpgradeAnimationState::ONGOING;
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeAnimationState UpgradeUnlockedHandler::UpdateHealthPotionUpgradeGained(const float)
{
    auto healthUpAnimationSoOpt = mScene.GetSceneObject(HEALTH_UP_ANIMATION_SO_NAME);
    if (healthUpAnimationSoOpt)
    {
        auto& healthUpAnimationSo = healthUpAnimationSoOpt->get();
        if (healthUpAnimationSo.mAnimation->VIsPaused())
        {
            mScene.RemoveAllSceneObjectsWithName(healthUpAnimationSo.mName);
            return UpgradeAnimationState::FINISHED;
        }
        else
        {
            return UpgradeAnimationState::ONGOING;
        }
    }
    
    return UpgradeAnimationState::FINISHED;
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeAnimationState UpgradeUnlockedHandler::UpdateMirrorImageUpgradeGained(const float dtMillis)
{
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto leftMirrorImageSoOpt = mScene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    auto rightMirrorImageSoOpt = mScene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    
    if (leftMirrorImageSoOpt && rightMirrorImageSoOpt && leftMirrorImageSoOpt->get().mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] < 0.5f)
    {
        auto& leftMirrorImageSo = leftMirrorImageSoOpt->get();
        auto& rightMirrorImageSo = rightMirrorImageSoOpt->get();
        
        bool shouldStartMirrorImageBulletFlow = false;
        
        leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
        if (leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 0.5f)
        {
            leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
            shouldStartMirrorImageBulletFlow = true;
        }
        
        rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
        if (rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 0.5f)
        {
            rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
            shouldStartMirrorImageBulletFlow = true;
        }
        
        if (shouldStartMirrorImageBulletFlow)
        {
            blueprint_flows::CreatePlayerBulletFlow(mFlows, mScene, mBox2dWorld);
            
            mFlows.emplace_back([&]()
            {
                mForceFinishAnimation = true;
            }, INTER_ANIMATION_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, ANIMATION_END_FLOW_NAME);

        }
    }
    if (playerSoOpt && mFlows.empty())
    {
        auto& playerSo = playerSoOpt->get();
        
        playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
        if (playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
        {
            playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            
            blueprint_flows::CreatePlayerBulletFlow(mFlows, mScene, mBox2dWorld, { game_constants::MIRROR_IMAGE_UGPRADE_NAME });
            
            mFlows.emplace_back([&]()
            {
                auto& resService = resources::ResourceLoadingService::GetInstance();
                
                auto playerOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
                if (playerOpt)
                {
                    {
                        SceneObject leftMirrorImageSo;
                        leftMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
                        leftMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
                        leftMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + LEFT_MIRROR_IMAGE_POSITION_OFFSET;
                        leftMirrorImageSo.mScale = LEFT_MIRROR_IMAGE_SCALE;
                        leftMirrorImageSo.mName = game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
                        leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                        mScene.AddSceneObject(std::move(leftMirrorImageSo));
                    }
                    
                    {
                        SceneObject rightMirrorImageSo;
                        rightMirrorImageSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::MIRROR_IMAGE_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
                        rightMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
                        rightMirrorImageSo.mPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
                        rightMirrorImageSo.mScale = RIGHT_MIRROR_IMAGE_SCALE;
                        rightMirrorImageSo.mName = game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
                        rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                        mScene.AddSceneObject(std::move(rightMirrorImageSo));
                    }
                }
            }, INTER_ANIMATION_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, MIRROR_IMAGE_FLOW_CREATION_NAME);
        }
    }
    
    return UpgradeAnimationState::ONGOING;
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeAnimationState UpgradeUnlockedHandler::UpdateDoubleBulletUpgradeGained(const float dtMillis)
{
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto leftMirrorImageSoOpt = mScene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    auto rightMirrorImageSoOpt = mScene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    
    if (playerSoOpt && mFlows.empty())
    {
        auto& playerSo = playerSoOpt->get();
        
        if (leftMirrorImageSoOpt)
        {
            auto& leftMirrorImageSo = leftMirrorImageSoOpt->get();
            leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
            if (leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 0.5f)
            {
                leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
            }
        }
    
        if (rightMirrorImageSoOpt)
        {
            auto& rightMirrorImageSo = rightMirrorImageSoOpt->get();
            rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
            if (rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 0.5f)
            {
                rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
            }
        }
        
        playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
        if (playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
        {
            playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            
            blueprint_flows::CreatePlayerBulletFlow(mFlows, mScene, mBox2dWorld, { game_constants::DOUBLE_BULLET_UGPRADE_NAME });
            
            mFlows.emplace_back([&]()
            {
                blueprint_flows::CreatePlayerBulletFlow(mFlows, mScene, mBox2dWorld);
                
                mFlows.emplace_back([&]()
                {
                    mForceFinishAnimation = true;
                }, INTER_ANIMATION_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, ANIMATION_END_FLOW_NAME);
                
            }, INTER_ANIMATION_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, DOUBLE_BULLET_FLOW_CREATION_NAME);
        }
    }
    
    return UpgradeAnimationState::ONGOING;
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedHandler::UpgradeAnimationState UpgradeUnlockedHandler::UpdatePlayerShieldUpgradeGained(const float dtMillis)
{
    auto playerSoOpt = mScene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto leftMirrorImageSoOpt = mScene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    auto rightMirrorImageSoOpt = mScene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
    
    if (playerSoOpt && mFlows.empty())
    {
        auto& playerSo = playerSoOpt->get();
        
        if (leftMirrorImageSoOpt)
        {
            auto& leftMirrorImageSo = leftMirrorImageSoOpt->get();
            leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
            if (leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 0.5f)
            {
                leftMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
            }
        }
    
        if (rightMirrorImageSoOpt)
        {
            auto& rightMirrorImageSo = rightMirrorImageSoOpt->get();
            rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
            if (rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 0.5f)
            {
                rightMirrorImageSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
            }
        }
        
        playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * SCENE_OBJECT_FADE_IN_ALPHA_SPEED;
        if (playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
        {
            playerSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        
            SceneObject playerShieldSo;
            
            auto& resService = resources::ResourceLoadingService::GetInstance();
            playerShieldSo.mAnimation = std::make_unique<PlayerShieldAnimation>(&playerShieldSo, resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PLAYER_SHIELD_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PLAYER_SHIELD_EFFECT_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + PLAYER_SHIELD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::PLAYER_SHIELD_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            playerShieldSo.mSceneObjectType = SceneObjectType::WorldGameObject;
            playerShieldSo.mPosition = math::Box2dVec2ToGlmVec3(playerSoOpt->get().mBody->GetWorldCenter()) + PLAYER_SHIELD_POSITION_OFFSET;
            playerShieldSo.mScale = PLAYER_SHIELD_SCALE;
            playerShieldSo.mName = game_constants::PLAYER_SHIELD_SCENE_OBJECT_NAME;
            
            playerShieldSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(playerShieldSo.mAnimation->VGetCurrentTextureResourceId(), playerShieldSo.mAnimation->VGetCurrentMeshResourceId(), playerShieldSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, PLAYER_SHIELD_ROTATION_SPEED, false));
            
            playerShieldSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(playerShieldSo.mAnimation->VGetCurrentTextureResourceId(), playerShieldSo.mAnimation->VGetCurrentMeshResourceId(), playerShieldSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.0f), PulsingAnimation::PulsingMode::PULSE_CONTINUALLY, 0.0f, PLAYER_PULSE_SHIELD_ANIM_SPEED, PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR, false));
            
            mScene.AddSceneObject(std::move(playerShieldSo));
            
            mFlows.emplace_back([&]()
            {
                mForceFinishAnimation = true;
            }, INTER_ANIMATION_DELAY_MILLIS, RepeatableFlow::RepeatPolicy::ONCE, ANIMATION_END_FLOW_NAME);
        }
    }
    
    return UpgradeAnimationState::ONGOING;
}

///------------------------------------------------------------------------------------------------
