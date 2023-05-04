///------------------------------------------------------------------------------------------------
///  SettingsMenuGameState.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "SettingsMenuGameState.h"
#include "../GameConstants.h"
#include "../GameSingletons.h"
#include "../Scene.h"
#include "../SceneObject.h"
#include "../SceneObjectUtils.h"
#include "../datarepos/FontRepository.h"
#include "../dataloaders/GUISceneLoader.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/Logging.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId SettingsMenuGameState::STATE_NAME("SettingsMenuGameState");

static const std::string SCENE_NAME = "settings_menu_scene";

static const strutils::StringId BACK_BUTTON_SO_NAME = strutils::StringId("back_button");
static const strutils::StringId ACCELEROMETER_INPUT_METHOD_SO_NAME = strutils::StringId("input_method_accelerometer");
static const strutils::StringId JOYSTICK_INPUT_METHOD_SO_NAME = strutils::StringId("input_method_joystick");

static const glm::vec4 DEFAULT_SETTING_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
static const glm::vec4 SELECTED_SETTING_COLOR = glm::vec4(0.0f, 0.81f, 1.0f, 1.0f);
static const float SETTINGS_MAX_DARKENING_VALUE = 0.95f;

///------------------------------------------------------------------------------------------------

void SettingsMenuGameState::VInitialize()
{
    mSceneElementIds.clear();
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    mScene->AddOverlayController(game_constants::FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED, SETTINGS_MAX_DARKENING_VALUE, true);
    
    GUISceneLoader loader;
    const auto& sceneDefinition = loader.LoadGUIScene(SCENE_NAME);
    
    for (const auto& guiElement: sceneDefinition.mGUIElements)
    {
        SceneObject guiSceneObject;
        guiSceneObject.mName = guiElement.mSceneObjectName;
        guiSceneObject.mPosition = guiElement.mPosition;
        guiSceneObject.mScale = guiElement.mScale;
        guiSceneObject.mText = guiElement.mText;
        guiSceneObject.mFontName = guiElement.mFontName;
        guiSceneObject.mInvisible = guiElement.mInvisible;
        guiSceneObject.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(guiElement.mTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), guiElement.mShaderResourceId, glm::vec3(1.0f), false);
        guiSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
        
        if (guiSceneObject.mFontName != strutils::StringId())
        {
            guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(guiSceneObject.mFontName)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), guiElement.mShaderResourceId, glm::vec3(1.0f), false);
        }
        
        mSceneElementIds.push_back(guiSceneObject.mName);
        mScene->AddSceneObject(std::move(guiSceneObject));
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective SettingsMenuGameState::VUpdate(const float dtMillis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& guiCamera = camOpt->get();
    const auto& inputContext = GameSingletons::GetInputContext();
    
    for (const auto& elementId: mSceneElementIds)
    {
        auto elementSoOpt = mScene->GetSceneObject(elementId);
        if (elementSoOpt)
        {
            elementSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].a += game_constants::TEXT_FADE_IN_ALPHA_SPEED * dtMillis;
            if (elementSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].a >= 1.0f)
            {
                elementSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].a = 1.0f;
            }
        }
    }
    
    UpdateSelectedSettingsColor();
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        auto backButtonSoOpt = mScene->GetSceneObject(BACK_BUTTON_SO_NAME);
        if (backButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(backButtonSoOpt->get(), touchPos))
        {
            mScene->ResumeOverlayController();
            GameSingletons::ConsumeInput();
            Complete();
        }
        
        auto accelInputMethodSoOpt = mScene->GetSceneObject(ACCELEROMETER_INPUT_METHOD_SO_NAME);
        if (accelInputMethodSoOpt && scene_object_utils::IsPointInsideSceneObject(accelInputMethodSoOpt->get(), touchPos))
        {
            GameSingletons::SetAccelerometerControl(true);
        }
        
        auto joystickInputMethodSoOpt = mScene->GetSceneObject(JOYSTICK_INPUT_METHOD_SO_NAME);
        if (joystickInputMethodSoOpt && scene_object_utils::IsPointInsideSceneObject(joystickInputMethodSoOpt->get(), touchPos))
        {
            GameSingletons::SetAccelerometerControl(false);
        }
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
}

///------------------------------------------------------------------------------------------------

void SettingsMenuGameState::VDestroy()
{
    for (auto elementId: mSceneElementIds)
    {
        mScene->RemoveAllSceneObjectsWithName(elementId);
    }
}

///------------------------------------------------------------------------------------------------

void SettingsMenuGameState::UpdateSelectedSettingsColor()
{
    auto accelInputMethodSoOpt = mScene->GetSceneObject(ACCELEROMETER_INPUT_METHOD_SO_NAME);
    auto joystickInputMethodSoOpt = mScene->GetSceneObject(JOYSTICK_INPUT_METHOD_SO_NAME);
    
    if (accelInputMethodSoOpt && joystickInputMethodSoOpt)
    {
        if (GameSingletons::GetAccelerometerControl())
        {
            accelInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].r = SELECTED_SETTING_COLOR.r;
            accelInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].g = SELECTED_SETTING_COLOR.g;
            accelInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].b = SELECTED_SETTING_COLOR.b;
            
            joystickInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].r = DEFAULT_SETTING_COLOR.r;
            joystickInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].g = DEFAULT_SETTING_COLOR.g;
            joystickInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].b = DEFAULT_SETTING_COLOR.b;
        }
        else
        {
            joystickInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].r = SELECTED_SETTING_COLOR.r;
            joystickInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].g = SELECTED_SETTING_COLOR.g;
            joystickInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].b = SELECTED_SETTING_COLOR.b;
            
            accelInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].r = DEFAULT_SETTING_COLOR.r;
            accelInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].g = DEFAULT_SETTING_COLOR.g;
            accelInputMethodSoOpt->get().mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME].b = DEFAULT_SETTING_COLOR.b;
        }
    }
    
}

///------------------------------------------------------------------------------------------------
