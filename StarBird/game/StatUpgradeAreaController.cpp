///------------------------------------------------------------------------------------------------
///  StatUpgradeAreaController.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "FontRepository.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "SceneObject.h"
#include "SceneObjectUtils.h"
#include "StatUpgradeAreaController.h"

///------------------------------------------------------------------------------------------------
// 2.93 7.32
// +button 2.80, 6.02
// -button 4.50, 6.02

static const strutils::StringId STAT_UPGRADE_BACKGROUND_NAME = strutils::StringId("STAT_UPGRADE_BACKGROUND");

///------------------------------------------------------------------------------------------------

StatUpgradeAreaController::StatUpgradeAreaController(Scene& scene, std::unique_ptr<BaseAnimation> statUpgradeBackgroundAnimation, const glm::vec3& position, const glm::vec3& scale, const std::string& text, const float initialStatValue, const float statIncrement, const bool floatDisplay)
    : mScene(scene)
    , mStatIncrement(statIncrement)
    , mInitialStatValue(initialStatValue)
    , mFloatDisplay(floatDisplay)
    , mStatValue(initialStatValue)
    , mCost(0)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    {
        SceneObject statUpgradeBackgroundSo;
        statUpgradeBackgroundSo.mPosition = position;
        statUpgradeBackgroundSo.mScale = scale;
        statUpgradeBackgroundSo.mAnimation = std::move(statUpgradeBackgroundAnimation);
        statUpgradeBackgroundSo.mSceneObjectType = SceneObjectType::GUIObject;
        statUpgradeBackgroundSo.mName = STAT_UPGRADE_BACKGROUND_NAME;
        mScene.AddSceneObject(std::move(statUpgradeBackgroundSo));
    }
    
    {
        SceneObject statTextSo;
        statTextSo.mPosition = glm::vec3(position.x - 1.0f, position.y + 1.03f, position.z + 0.5f);
        statTextSo.mScale = glm::vec3(0.0067f, 0.0067f, position.z);
        statTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), statTextSo.mScale, false);
        statTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        statTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        statTextSo.mText = text;
        mScene.AddSceneObject(std::move(statTextSo));
    }
    
    {
        SceneObject costTextSo;
        costTextSo.mPosition = glm::vec3(position.x - 1.0f, position.y + 0.2f, position.z + 0.5f);
        costTextSo.mScale = glm::vec3(0.0067f, 0.0067f, position.z);
        costTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), costTextSo.mScale, false);
        costTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        costTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        costTextSo.mText = "COST ";
        mScene.AddSceneObject(std::move(costTextSo));
    }
    
    {
        SceneObject statValueSo;
        statValueSo.mPosition = glm::vec3(position.x + 1.8f, position.y + 1.03f, position.z + 0.5f);
        statValueSo.mScale = glm::vec3(0.0067f, 0.0067f, position.z);
        statValueSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), statValueSo.mScale, false);
        statValueSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        statValueSo.mSceneObjectType = SceneObjectType::GUIObject;
        
        mStatValueTextName = strutils::StringId(text + "STAT_VALUE");
        statValueSo.mName = mStatValueTextName;
        
        std::stringstream statValueString;
        if (mFloatDisplay)
        {
            statValueString << std::fixed << std::setprecision(1) << mStatValue;
        }
        else
        {
            statValueString << static_cast<int>(mStatValue);
        }
        
        statValueSo.mText = statValueString.str();
        if (statValueSo.mText.size() == 1)
        {
            statValueSo.mText = " " + statValueSo.mText;
        }
        mScene.AddSceneObject(std::move(statValueSo));
    }
    
    {
        SceneObject costValueSo;
        costValueSo.mPosition = glm::vec3(position.x + 1.0f, position.y + 0.2f, position.z + 0.5f);
        costValueSo.mScale = glm::vec3(0.0067f, 0.0067f, position.z);
        costValueSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), costValueSo.mScale, false);
        costValueSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        costValueSo.mSceneObjectType = SceneObjectType::GUIObject;
        
        mUpgradeCostTextName = strutils::StringId(text + "UPGRADE_COST");
        costValueSo.mName = mUpgradeCostTextName;
        
        costValueSo.mText = std::to_string(mCost);
        if (costValueSo.mText.size() == 1)
        {
            costValueSo.mText = " " + costValueSo.mText;
        }
        mScene.AddSceneObject(std::move(costValueSo));
    }
    
    {
        SceneObject plusButtonSo;
        plusButtonSo.mPosition = glm::vec3(position.x - 0.13f, position.y - 1.3f, position.z + 0.5f);
        plusButtonSo.mScale = glm::vec3(1.25f);
        plusButtonSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "plus_button_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), plusButtonSo.mScale, false);
        
        mPlusButtonName = strutils::StringId(text + "PLUS_BUTTON");
        plusButtonSo.mName = mPlusButtonName;
        plusButtonSo.mSceneObjectType = SceneObjectType::GUIObject;
        mScene.AddSceneObject(std::move(plusButtonSo));
    }
    
    {
        SceneObject minusButtonSo;
        minusButtonSo.mPosition = glm::vec3(position.x + 1.57f, position.y - 1.3f, position.z + 0.5f);
        minusButtonSo.mScale = glm::vec3(1.25f);
        minusButtonSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "minus_button_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), minusButtonSo.mScale, false);
        minusButtonSo.mSceneObjectType = SceneObjectType::GUIObject;
        
        mMinusButtonName = strutils::StringId(text + "MINUS_BUTTON");
        minusButtonSo.mName = mMinusButtonName;
        minusButtonSo.mInvisible = true;
        mScene.AddSceneObject(std::move(minusButtonSo));
    }
    
    {
        SceneObject crystalIconSo;
        crystalIconSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        crystalIconSo.mPosition = glm::vec3(position.x + 2.0f, position.y + 0.52f, position.z + 0.5f);
        mTargetCrystalPosition = crystalIconSo.mPosition;
        crystalIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalIconSo.mScale = glm::vec3(0.3f, 0.3f, 0.3f);
        crystalIconSo.mName = game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(crystalIconSo));
    }
}

///------------------------------------------------------------------------------------------------

float StatUpgradeAreaController::GetCurrentStatValue() const
{
    return mStatValue;
}

///------------------------------------------------------------------------------------------------

float StatUpgradeAreaController::GetCurrentCost() const
{
    return mCost;
}

///------------------------------------------------------------------------------------------------

const glm::vec3& StatUpgradeAreaController::GetTargetCrystalPosition() const
{
    return mTargetCrystalPosition;
}

///------------------------------------------------------------------------------------------------

void StatUpgradeAreaController::Update(const float dtMillis, const float currentTotalCost)
{
    auto& inputContext = GameSingletons::GetInputContext();
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    auto& camera = camOpt->get();
    
    glm::vec3 originalFingerDownTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, camera.GetViewMatrix(), camera.GetProjMatrix());
    
    auto plusButtonSoOpt = mScene.GetSceneObject(mPlusButtonName);
    auto minusButtonSoOpt = mScene.GetSceneObject(mMinusButtonName);
    
    if (plusButtonSoOpt)
    {
        auto& plusButtonSo = plusButtonSoOpt->get();
        plusButtonSo.mInvisible = currentTotalCost >= GameSingletons::GetCrystalCount();
        
        if (!plusButtonSo.mInvisible && inputContext.mEventType == SDL_FINGERDOWN && mLastInputContextEventType != SDL_FINGERDOWN && scene_object_utils::IsPointInsideSceneObject(plusButtonSo, originalFingerDownTouchPos))
        {
            plusButtonSo.mScale = glm::vec3(1.25f);
            plusButtonSo.mAnimation = std::make_unique<PulsingAnimation>(plusButtonSo.mAnimation->VGetCurrentTextureResourceId(), plusButtonSo.mAnimation->VGetCurrentMeshResourceId(), plusButtonSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.25f), PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, 0.02f, 1.0f/50.0f, false);
            
            mStatValue += mStatIncrement;
            mCost++;
        }
    }
    
    if (minusButtonSoOpt)
    {
        auto& minusButtonSo = minusButtonSoOpt->get();
        minusButtonSo.mInvisible = mStatValue <= mInitialStatValue;
        
        if (!minusButtonSo.mInvisible && inputContext.mEventType == SDL_FINGERDOWN && mLastInputContextEventType != SDL_FINGERDOWN && scene_object_utils::IsPointInsideSceneObject(minusButtonSo, originalFingerDownTouchPos))
        {
            minusButtonSo.mScale = glm::vec3(1.25f);
            minusButtonSo.mAnimation = std::make_unique<PulsingAnimation>(minusButtonSo.mAnimation->VGetCurrentTextureResourceId(), minusButtonSo.mAnimation->VGetCurrentMeshResourceId(), minusButtonSo.mAnimation->VGetCurrentShaderResourceId(), glm::vec3(1.25f), PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, 0.02f, 1.0f/50.0f, false);
            
            mStatValue -= mStatIncrement;
            mCost--;
        }
    }
    
    auto statValueTextSoOpt = mScene.GetSceneObject(mStatValueTextName);
    if (statValueTextSoOpt)
    {
        auto& statValueSo = statValueTextSoOpt->get();
        
        std::stringstream statValueString;
        if (mFloatDisplay)
        {
            statValueString << std::fixed << std::setprecision(1) << mStatValue;
        }
        else
        {
            statValueString << static_cast<int>(mStatValue);
        }
        
        statValueSo.mText = statValueString.str();
        if (statValueSo.mText.size() == 1)
        {
            statValueSo.mText = " " + statValueSo.mText;
        }
    }
    
    auto upgradeCostTextSoOpt = mScene.GetSceneObject(mUpgradeCostTextName);
    if (upgradeCostTextSoOpt)
    {
        auto& upgradeCostSo = upgradeCostTextSoOpt->get();
        
        upgradeCostSo.mText = std::to_string(mCost);
        if (upgradeCostSo.mText.size() == 1)
        {
            upgradeCostSo.mText = " " + upgradeCostSo.mText;
        }
    }
    
    mLastInputContextEventType = inputContext.mEventType;
}

///------------------------------------------------------------------------------------------------
