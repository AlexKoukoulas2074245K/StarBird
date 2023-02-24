///------------------------------------------------------------------------------------------------
///  DebugConsoleGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "DebugConsoleGameState.h"
#include "../GameObjectConstants.h"
#include "../GameSingletons.h"
#include "../Scene.h"
#include "../SceneObject.h"
#include "../SceneObjectConstants.h"
#include "../SceneObjectUtils.h"
#include "../datarepos/FontRepository.h"
#include "../dataloaders/GUISceneLoader.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/Logging.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId DebugConsoleGameState::STATE_NAME("DebugConsoleGameState");

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::VInitialize()
{
    mSceneElementIds.clear();
    mPastCommandElementIds.clear();
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
        overlaySo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME));
        overlaySo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mCustomScale = game_object_constants::FULL_SCREEN_OVERLAY_SCALE;
        overlaySo.mCustomPosition = game_object_constants::FULL_SCREEN_OVERLAY_POSITION;
        overlaySo.mNameTag = scene_object_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME;
        overlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        
        mSceneElementIds.push_back(overlaySo.mNameTag);
        mScene->AddSceneObject(std::move(overlaySo));
    }
    
    GUISceneLoader loader;
    const auto& sceneDefinition = loader.LoadGUIScene("debug_console");
    
    for (const auto& guiElement: sceneDefinition.mGUIElements)
    {
        SceneObject guiSceneObject;
        guiSceneObject.mNameTag = guiElement.mSceneObjectName;
        guiSceneObject.mCustomPosition = guiElement.mPosition;
        guiSceneObject.mCustomScale = guiElement.mScale;
        guiSceneObject.mText = guiElement.mText;
        guiSceneObject.mFontName = guiElement.mFontName;
        guiSceneObject.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        guiSceneObject.mShaderResourceId = guiElement.mShaderResourceId;
        guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(guiElement.mTextureResourceId);
        guiSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
        
        if (guiSceneObject.mFontName != strutils::StringId())
        {
            guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(guiSceneObject.mFontName)->get().mFontTextureResourceId);
        }
        
        mSceneElementIds.push_back(guiSceneObject.mNameTag);
        mScene->AddSceneObject(std::move(guiSceneObject));
    }
    
    GameSingletons::SetInputContextText("");
    SDL_StartTextInput();
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective DebugConsoleGameState::VUpdate(const float dtMillis)
{
    auto overlaySoOpt = mScene->GetSceneObject(scene_object_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME);
    if (overlaySoOpt)
    {
        auto& overlaySo = overlaySoOpt->get();
        auto& overlayAlpha = overlaySo.mShaderFloatUniformValues[scene_object_constants::CUSTOM_ALPHA_UNIFORM_NAME];
        overlayAlpha += dtMillis * game_object_constants::FULL_SCREEN_OVERLAY_DARKENING_SPEED;
        if (overlayAlpha >= game_object_constants::FULL_SCREEN_OVERLAY_MAX_ALPHA)
        {
            overlayAlpha = game_object_constants::FULL_SCREEN_OVERLAY_MAX_ALPHA;
        }
    }
    
    auto commandTextSoOpt = mScene->GetSceneObject(scene_object_constants::DEBUG_COMMAND_TEXT_SCENE_OBJECT_NAME);
    if (commandTextSoOpt)
    {
        commandTextSoOpt->get().mText = GameSingletons::GetInputContext().mText;
        
        if (GameSingletons::GetInputContext().mEventType == SDL_KEYDOWN && GameSingletons::GetInputContext().mKeyCode == SDL_SCANCODE_RETURN)
        {
            ExecuteCommand(GameSingletons::GetInputContext().mText, commandTextSoOpt->get());
        }
    }
    
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& guiCamera = camOpt->get();
    const auto& inputContext = GameSingletons::GetInputContext();
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        auto continueButtonSoOpt = mScene->GetSceneObject(scene_object_constants::DEBUG_BACK_TO_GAME_SCENE_OBJECT_NAME);
        if (continueButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(continueButtonSoOpt->get(), touchPos))
        {
            SDL_StopTextInput();
            GameSingletons::ConsumeInput();
            Complete();
        }
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::VDestroy()
{
    for (const auto& elementId: mSceneElementIds)
    {
        mScene->RemoveAllSceneObjectsWithNameTag(elementId);
    }
    
    for (const auto& elementId: mPastCommandElementIds)
    {
        mScene->RemoveAllSceneObjectsWithNameTag(elementId);
    }
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::ExecuteCommand(const std::string& command, const SceneObject& commandTextSo)
{
    auto commandComponents = strutils::StringSplit(command, ' ');
    
    PostCommandExecution(command, commandTextSo);
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::PostCommandExecution(const std::string& command, const SceneObject& commandTextSo)
{
    // Create a past command SO out of current executed command
    {
        SceneObject pastTextSceneObject;
        pastTextSceneObject.mNameTag = strutils::StringId(std::to_string(mPastCommandElementIds.size()));
        pastTextSceneObject.mCustomPosition = commandTextSo.mCustomPosition;
        pastTextSceneObject.mCustomPosition.x += scene_object_constants::DEBUG_PAST_COMMAND_X_OFFSET;
        pastTextSceneObject.mCustomScale = commandTextSo.mCustomScale;
        pastTextSceneObject.mText = commandTextSo.mText;
        pastTextSceneObject.mFontName = commandTextSo.mFontName;
        pastTextSceneObject.mMeshResourceId = commandTextSo.mMeshResourceId;
        pastTextSceneObject.mShaderResourceId = commandTextSo.mShaderResourceId;
        pastTextSceneObject.mAnimation = commandTextSo.mAnimation->VClone();
        pastTextSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
        mPastCommandElementIds.push_back(pastTextSceneObject.mNameTag);
        mScene->AddSceneObject(std::move(pastTextSceneObject));
    }
    
    // Push all past commands up
    for (const auto& id: mPastCommandElementIds)
    {
        auto soOpt = mScene->GetSceneObject(id);
        if (soOpt)
        {
            soOpt->get().mCustomPosition.y += scene_object_constants::DEBUG_PAST_COMMAND_Y_OFFSET;
        }
    }
    
    GameSingletons::ConsumeInput();
    GameSingletons::SetInputContextText("");
}

///------------------------------------------------------------------------------------------------
