///------------------------------------------------------------------------------------------------
///  StatUpgradeAreaController.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "FontRepository.h"
#include "GameConstants.h"
#include "Scene.h"
#include "SceneObject.h"
#include "StatUpgradeAreaController.h"

///------------------------------------------------------------------------------------------------

static const strutils::StringId STAT_UPGRADE_BACKGROUND_NAME = strutils::StringId("STAT_UPGRADE_BACKGROUND");

///------------------------------------------------------------------------------------------------

StatUpgradeAreaController::StatUpgradeAreaController(Scene& scene, std::unique_ptr<BaseAnimation> statUpgradeBackgroundAnimation, const glm::vec3& position, const glm::vec3& scale, const std::string& text, const float initialStatValue)
    : mScene(scene)
    , mStatValue(initialStatValue)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Stat Upgrade area background
    SceneObject statUpgradeBackgroundSo;
    statUpgradeBackgroundSo.mPosition = position;
    statUpgradeBackgroundSo.mScale = scale;
    statUpgradeBackgroundSo.mAnimation = std::move(statUpgradeBackgroundAnimation);
    statUpgradeBackgroundSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    statUpgradeBackgroundSo.mName = STAT_UPGRADE_BACKGROUND_NAME;
    mScene.AddSceneObject(std::move(statUpgradeBackgroundSo));
    
    SceneObject textSo;
    textSo.mPosition = glm::vec3(position.x + 0.5f, position.y + 0.5f, position.z + 0.5f);
    textSo.mScale = glm::vec3(0.006f, 0.006f, position.z);
    textSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(textSo.mScale), false);
    textSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
    textSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    textSo.mText = text;
    mScene.AddSceneObject(std::move(textSo));
    
    
    SceneObject plusButtonSo;
    plusButtonSo.mPosition = glm::vec3(position.x - 1.0f, position.y + 2.5f, position.z + 0.5f);
    plusButtonSo.mScale = glm::vec3(1.0f);
    plusButtonSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "plus_button_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(textSo.mScale), false);
    plusButtonSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    mScene.AddSceneObject(std::move(plusButtonSo));
    
    SceneObject minusButtonSo;
    minusButtonSo.mPosition = glm::vec3(position.x + 1.0f, position.y + 2.5f, position.z + 0.5f);
    minusButtonSo.mScale = glm::vec3(1.0f);
    minusButtonSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "minus_button_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(textSo.mScale), false);
    minusButtonSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    mScene.AddSceneObject(std::move(minusButtonSo));
}
