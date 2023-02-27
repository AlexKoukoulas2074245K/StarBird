///------------------------------------------------------------------------------------------------
///  DebugConsoleGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "DebugConsoleGameState.h"
#include "../GameObjectConstants.h"
#include "../GameSingletons.h"
#include "../PhysicsConstants.h"
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
const glm::vec4 DebugConsoleGameState::SUCCESS_COLOR(0.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 DebugConsoleGameState::FAILURE_COLOR(1.0f, 0.0f, 0.0f, 1.0f);
const float DebugConsoleGameState::BIRDS_EYE_VIEW_CAMERA_LENSE_HEIGHT = 90.0f;

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::VInitialize()
{
    RegisterCommands();
    mSceneElementIds.clear();
    mPastCommandElementIds.clear();
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_COLOR_SHADER_FILE_NAME);
    
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
    
    bool completeButtonPressed = false;
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        auto continueButtonSoOpt = mScene->GetSceneObject(scene_object_constants::DEBUG_BACK_TO_GAME_SCENE_OBJECT_NAME);
        if (continueButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(continueButtonSoOpt->get(), touchPos))
        {
            completeButtonPressed = true;
        }
    }
    
    if (completeButtonPressed || inputContext.mKeyCode == SDL_SCANCODE_ESCAPE)
    {
        SDL_StopTextInput();
        GameSingletons::ConsumeInput();
        Complete();
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

void DebugConsoleGameState::RegisterCommands()
{
    mCommandMap[strutils::StringId("physx")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("physx on|off");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        else if (commandComponents[1] != "on" && commandComponents[1] != "off")
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        mScene->SetSceneRendererPhysicsDebugMode(commandComponents[1] == "on");
        return CommandExecutionResult(true, "Physics Debug turned " + commandComponents[1]);
    };
    
    mCommandMap[strutils::StringId("bov")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("bov on|off");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        else if (commandComponents[1] != "on" && commandComponents[1] != "off")
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        mScene->RemoveAllSceneObjectsWithNameTag(scene_object_constants::WALL_SCENE_OBJECT_NAME);
        
        if (commandComponents[1] == "on")
        {
            mPreviousCameraLenseHeight = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get().GetCameraLenseHeight();
            
            GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera(BIRDS_EYE_VIEW_CAMERA_LENSE_HEIGHT));
            mScene->CreateLevelWalls(GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject)->get(), false);
        }
        else if (mPreviousCameraLenseHeight > 0.0f)
        {
            GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera(mPreviousCameraLenseHeight));
            mScene->CreateLevelWalls(GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get(), true);
        }
        
        return CommandExecutionResult(true, "Bird's Eye View turned " + commandComponents[1]);
    };
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::ExecuteCommand(const std::string& command, const SceneObject& commandTextSo)
{
    auto commandComponents = strutils::StringSplit(command, ' ');
    
    if (!commandComponents.empty())
    {
        auto commandIter = mCommandMap.find(strutils::StringId(commandComponents[0]));
        if (commandIter != mCommandMap.end())
        {
            auto commandExecutionResult = mCommandMap[commandIter->first](commandComponents);
            SetCommandExecutionOutput(commandExecutionResult);
        }
        else
        {
            SetCommandExecutionOutput(CommandExecutionResult(false, "Invalid command"));
        }
        
        PostCommandExecution(command, commandTextSo);
    }
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::SetCommandExecutionOutput(const CommandExecutionResult& executionResult)
{
    auto commandOutputSoOpt = mScene->GetSceneObject(scene_object_constants::DEBUG_COMMAND_OUTPUT_SCENE_OBJECT_NAME);
    if (commandOutputSoOpt)
    {
        auto& commandOutputSo = commandOutputSoOpt->get();
        commandOutputSo.mText = executionResult.mMessage;
        commandOutputSo.mShaderResourceId = resources::ResourceLoadingService::GetInstance().GetResourceIdFromPath(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::CUSTOM_COLOR_SHADER_FILE_NAME);
        commandOutputSo.mShaderFloatVec4UniformValues[scene_object_constants::CUSTOM_COLOR_UNIFORM_NAME] = executionResult.mSuccess ? SUCCESS_COLOR : FAILURE_COLOR;
    }
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
