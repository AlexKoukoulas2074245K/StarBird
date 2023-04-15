///------------------------------------------------------------------------------------------------
///  UpgradeUnlockedAnimationHandler.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "UpgradeUnlockedAnimationHandler.h"
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

UpgradeUnlockedAnimationHandler::UpgradeUnlockedAnimationHandler(Scene& scene)
    : mScene(scene)
{
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedAnimationHandler::OnUpgradeGained(const strutils::StringId& upgradeId)
{
    mCurrentUpgradeNameUnlocked = upgradeId;
    
    auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
    auto& availableUpgrades = GameSingletons::GetAvailableUpgrades();
    
    equippedUpgrades[mCurrentUpgradeNameUnlocked] = availableUpgrades.at(mCurrentUpgradeNameUnlocked);
    availableUpgrades.erase(mCurrentUpgradeNameUnlocked);
    
    if (mCurrentUpgradeNameUnlocked == game_constants::CRYSTALS_GIFT_UGPRADE_NAME)
    {
        OnCrystalGiftUpgradeGained();
    }
}

///------------------------------------------------------------------------------------------------

UpgradeUnlockedAnimationHandler::UpgradeAnimationState UpgradeUnlockedAnimationHandler::Update(const float dtMillis)
{
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
    
    if (mCurrentUpgradeNameUnlocked == game_constants::CRYSTALS_GIFT_UGPRADE_NAME)
    {
        return UpdateCrystalGiftUpgradeGained();
    }
    else
    {
        return UpgradeAnimationState::FINISHED;
    }
}

///------------------------------------------------------------------------------------------------

void UpgradeUnlockedAnimationHandler::OnCrystalGiftUpgradeGained()
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

UpgradeUnlockedAnimationHandler::UpgradeAnimationState UpgradeUnlockedAnimationHandler::UpdateCrystalGiftUpgradeGained()
{
    return mCreatedSceneObjectNames.size() == 0 ? UpgradeAnimationState::FINISHED : UpgradeAnimationState::ONGOING;
}

///------------------------------------------------------------------------------------------------
