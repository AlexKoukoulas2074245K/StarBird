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

static const char* STAT_UPGRADE_BACKGROUND_NAME = "STAT_UPGRADE_BACKGROUND";

static const glm::vec4 STAT_UPGRADED_TEXT_COLOR = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

static const glm::vec3 STAT_TEXT_SCALE = glm::vec3(0.0067f, 0.0067f, 0.0067);
static const glm::vec3 STAT_DESCRIPTION_TEXT_OFFSET = glm::vec3(-1.0f, 1.03f, 0.5f);
static const glm::vec3 COST_DESCRIPTION_TEXT_OFFSET = glm::vec3(-1.0f, 0.2f, 0.5f);
static const glm::vec3 STAT_VALUE_TEXT_OFFSET = glm::vec3(1.8f, 1.03f, 0.5f);
static const glm::vec3 COST_VALUE_TEXT_OFFSET = glm::vec3(1.0f, 0.2f, 0.5f);
static const glm::vec3 PLUS_BUTTON_OFFSET = glm::vec3(-0.13f, -1.3f, 0.5f);
static const glm::vec3 MINUS_BUTTON_OFFSET = glm::vec3(1.57f, -1.3f, 0.5f);
static const glm::vec3 CRYSTAL_ICON_OFFSET = glm::vec3(2.0f, 0.52f, 0.5f);

static const glm::vec3 CONTROL_BUTTON_SCALE = glm::vec3(1.25f);
static const glm::vec3 CRYSTAL_ICON_SCALE = glm::vec3(0.3f);

static const float CONTROL_BUTTON_PULSING_SPEED = 0.02f;
static const float CONTROL_BUTTON_PULSING_ENLARGMENT_FACTOR = 1.0f/50.0f;

static const float STAT_UPGRADED_SCALE_MULTIPLIER = 0.9f;

static const int STAT_UPGRADE_COST = 10;

///------------------------------------------------------------------------------------------------

StatUpgradeAreaController::StatUpgradeAreaController(Scene& scene, std::unique_ptr<BaseAnimation> statUpgradeBackgroundAnimation, const glm::vec3& position, const glm::vec3& additionalOffsetForContainedSceneObjects, const glm::vec3& scale, const std::string& text, const float initialStatValue, const float statIncrement, const bool floatDisplay)
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
        statUpgradeBackgroundSo.mName = strutils::StringId(text + STAT_UPGRADE_BACKGROUND_NAME);
        mScene.AddSceneObject(std::move(statUpgradeBackgroundSo));
    }
    
    {
        SceneObject statDescriptionTextSo;
        statDescriptionTextSo.mPosition = position + STAT_DESCRIPTION_TEXT_OFFSET + additionalOffsetForContainedSceneObjects;
        statDescriptionTextSo.mScale = STAT_TEXT_SCALE;
        statDescriptionTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), STAT_TEXT_SCALE, false);
        statDescriptionTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        statDescriptionTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        statDescriptionTextSo.mText = text;
        mScene.AddSceneObject(std::move(statDescriptionTextSo));
    }
    
    {
        SceneObject costDescriptionTextSo;
        costDescriptionTextSo.mPosition = position + COST_DESCRIPTION_TEXT_OFFSET + additionalOffsetForContainedSceneObjects;
        costDescriptionTextSo.mScale = STAT_TEXT_SCALE;
        costDescriptionTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), STAT_TEXT_SCALE, false);
        costDescriptionTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        costDescriptionTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        costDescriptionTextSo.mText = "COST ";
        mScene.AddSceneObject(std::move(costDescriptionTextSo));
    }
    
    {
        SceneObject statValueTextSo;
        statValueTextSo.mPosition = position + STAT_VALUE_TEXT_OFFSET + additionalOffsetForContainedSceneObjects;
        statValueTextSo.mScale = STAT_TEXT_SCALE;
        statValueTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), STAT_TEXT_SCALE, false);
        statValueTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        statValueTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        
        mStatValueTextName = strutils::StringId(text + "STAT_VALUE");
        statValueTextSo.mName = mStatValueTextName;
        
        std::stringstream statValueString;
        if (mFloatDisplay)
        {
            statValueString << std::fixed << std::setprecision(1) << mStatValue;
        }
        else
        {
            statValueString << static_cast<int>(mStatValue);
        }
        
        statValueTextSo.mText = statValueString.str();
        if (statValueTextSo.mText.size() == 1)
        {
            statValueTextSo.mText = " " + statValueTextSo.mText;
        }
        mScene.AddSceneObject(std::move(statValueTextSo));
    }
    
    {
        SceneObject costValueTextSo;
        costValueTextSo.mPosition = position + COST_VALUE_TEXT_OFFSET + additionalOffsetForContainedSceneObjects;
        costValueTextSo.mScale = STAT_TEXT_SCALE;
        costValueTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), STAT_TEXT_SCALE, false);
        costValueTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        costValueTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        
        mUpgradeCostTextName = strutils::StringId(text + "UPGRADE_COST");
        costValueTextSo.mName = mUpgradeCostTextName;
        
        costValueTextSo.mText = std::to_string(mCost);
        if (costValueTextSo.mText.size() == 1)
        {
            costValueTextSo.mText = " " + costValueTextSo.mText;
        }
        mScene.AddSceneObject(std::move(costValueTextSo));
    }
    
    {
        SceneObject plusButtonSo;
        plusButtonSo.mPosition = position + PLUS_BUTTON_OFFSET + additionalOffsetForContainedSceneObjects;
        plusButtonSo.mScale = CONTROL_BUTTON_SCALE;
        plusButtonSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "plus_button_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), plusButtonSo.mScale, false);
        
        mPlusButtonName = strutils::StringId(text + "PLUS_BUTTON");
        plusButtonSo.mName = mPlusButtonName;
        plusButtonSo.mSceneObjectType = SceneObjectType::GUIObject;
        mScene.AddSceneObject(std::move(plusButtonSo));
    }
    
    {
        SceneObject minusButtonSo;
        minusButtonSo.mPosition = position + MINUS_BUTTON_OFFSET + additionalOffsetForContainedSceneObjects;
        minusButtonSo.mScale = CONTROL_BUTTON_SCALE;
        minusButtonSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "minus_button_mm.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), minusButtonSo.mScale, false);
        minusButtonSo.mSceneObjectType = SceneObjectType::GUIObject;
        
        mMinusButtonName = strutils::StringId(text + "MINUS_BUTTON");
        minusButtonSo.mName = mMinusButtonName;
        minusButtonSo.mInvisible = true;
        mScene.AddSceneObject(std::move(minusButtonSo));
    }
    
    {
        SceneObject crystalIconSo;
        crystalIconSo.mPosition = position + CRYSTAL_ICON_OFFSET + additionalOffsetForContainedSceneObjects;
        crystalIconSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        mTargetCrystalPosition = crystalIconSo.mPosition;
        crystalIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalIconSo.mScale = CRYSTAL_ICON_SCALE;
        crystalIconSo.mName = strutils::StringId(text + game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME.GetString());
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
            plusButtonSo.mScale = CONTROL_BUTTON_SCALE;
            plusButtonSo.mAnimation = std::make_unique<PulsingAnimation>(plusButtonSo.mAnimation->VGetCurrentTextureResourceId(), plusButtonSo.mAnimation->VGetCurrentMeshResourceId(), plusButtonSo.mAnimation->VGetCurrentShaderResourceId(), CONTROL_BUTTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONTROL_BUTTON_PULSING_SPEED, CONTROL_BUTTON_PULSING_ENLARGMENT_FACTOR, false);
            
            mStatValue += mStatIncrement;
            mCost += STAT_UPGRADE_COST;
        }
    }
    
    if (minusButtonSoOpt)
    {
        auto& minusButtonSo = minusButtonSoOpt->get();
        minusButtonSo.mInvisible = mStatValue <= mInitialStatValue;
        
        if (!minusButtonSo.mInvisible && inputContext.mEventType == SDL_FINGERDOWN && mLastInputContextEventType != SDL_FINGERDOWN && scene_object_utils::IsPointInsideSceneObject(minusButtonSo, originalFingerDownTouchPos))
        {
            minusButtonSo.mScale = CONTROL_BUTTON_SCALE;
            minusButtonSo.mAnimation = std::make_unique<PulsingAnimation>(minusButtonSo.mAnimation->VGetCurrentTextureResourceId(), minusButtonSo.mAnimation->VGetCurrentMeshResourceId(), minusButtonSo.mAnimation->VGetCurrentShaderResourceId(), CONTROL_BUTTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONTROL_BUTTON_PULSING_SPEED, CONTROL_BUTTON_PULSING_ENLARGMENT_FACTOR, false);
            
            mStatValue -= mStatIncrement;
            mCost -= STAT_UPGRADE_COST;
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
        
        if (mStatValue > mInitialStatValue)
        {
            statValueSo.mScale = STAT_TEXT_SCALE * STAT_UPGRADED_SCALE_MULTIPLIER;
            statValueSo.mAnimation = std::make_unique<SingleFrameAnimation>(statValueSo.mAnimation->VGetCurrentTextureResourceId(), statValueSo.mAnimation->VGetCurrentMeshResourceId(), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME), statValueSo.mScale, false);
            statValueSo.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = STAT_UPGRADED_TEXT_COLOR;
        }
        else
        {
            statValueSo.mScale = STAT_TEXT_SCALE;
            statValueSo.mAnimation = std::make_unique<SingleFrameAnimation>(statValueSo.mAnimation->VGetCurrentTextureResourceId(), statValueSo.mAnimation->VGetCurrentMeshResourceId(), resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), STAT_TEXT_SCALE, false);
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
