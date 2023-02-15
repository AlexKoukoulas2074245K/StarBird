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
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"


///------------------------------------------------------------------------------------------------

UpgradesLogicHandler::UpgradesLogicHandler(Scene& scene, LevelUpdater& levelUpdater)
    : mScene(scene)
    , mLevelUpdater(levelUpdater)
{
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::OnUpgradeEquipped(const strutils::StringId& upgradeId)
{
    if (upgradeId == gameobject_constants::MIRROR_IMAGE_UGPRADE_NAME)
    {
        CreateMirrorImageSceneObjects();
    }
    else if (upgradeId == gameobject_constants::BULLET_SPEED_UGPRADE_NAME)
    {
        auto bulletFlowOpt = mLevelUpdater.GetFlow(gameobject_constants::PLAYER_BULLET_FLOW_NAME);
        if (bulletFlowOpt)
        {
            bulletFlowOpt->get().SetDuration(gameobject_constants::PLAYER_BULLET_FLOW_DELAY_MILLIS/2);
        }
    }
    else if (upgradeId == gameobject_constants::PLAYER_SPEED_UGPRADE_NAME)
    {
        auto playerTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetMutableObjectTypeDefinition(gameobject_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        if (playerTypeDefOpt)
        {
            playerTypeDefOpt->get().mSpeed *= 1.5f;
        }
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::OnUpdate(const float dtMillis)
{
    auto& equippedUpgrades = GameSingletons::GetEquippedUpgrades();
    bool mirrorImageEquipped = equippedUpgrades.count(gameobject_constants::MIRROR_IMAGE_UGPRADE_NAME) != 0;
    
    // Update Mirror Images
    if (mirrorImageEquipped)
    {
        auto playerSoOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
        auto leftMirrorImageSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
        auto rightMirrorImageSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
        
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
            
            leftMirrorImageSo.mCustomPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + gameobject_constants::LEFT_MIRROR_IMAGE_POSITION_OFFSET;
            rightMirrorImageSo.mCustomPosition = math::Box2dVec2ToGlmVec3(playerSo.mBody->GetWorldCenter()) + gameobject_constants::RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
        }
    }
}

///------------------------------------------------------------------------------------------------

void UpgradesLogicHandler::CreateMirrorImageSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    {
        SceneObject leftMirrorImageSo;
        leftMirrorImageSo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
        leftMirrorImageSo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::PLAYER_TEXTURE_FILE_NAME);
        leftMirrorImageSo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        leftMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        leftMirrorImageSo.mCustomPosition = gameobject_constants::LEFT_MIRROR_IMAGE_POSITION_OFFSET;
        leftMirrorImageSo.mCustomScale = gameobject_constants::LEFT_MIRROR_IMAGE_SCALE;
        leftMirrorImageSo.mNameTag = sceneobject_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
        leftMirrorImageSo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
        mScene.AddSceneObject(std::move(leftMirrorImageSo));
    }
    
    {
        SceneObject rightMirrorImageSo;
        rightMirrorImageSo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
        rightMirrorImageSo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::PLAYER_TEXTURE_FILE_NAME);
        rightMirrorImageSo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        rightMirrorImageSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        rightMirrorImageSo.mCustomPosition = gameobject_constants::RIGHT_MIRROR_IMAGE_POSITION_OFFSET;
        rightMirrorImageSo.mCustomScale = gameobject_constants::RIGHT_MIRROR_IMAGE_SCALE;
        rightMirrorImageSo.mNameTag = sceneobject_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME;
        rightMirrorImageSo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.5f;
        mScene.AddSceneObject(std::move(rightMirrorImageSo));
    }
}

///------------------------------------------------------------------------------------------------
