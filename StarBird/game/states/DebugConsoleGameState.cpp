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

///------------------------------------------------------------------------------------------------

const strutils::StringId DebugConsoleGameState::STATE_NAME("DebugConsoleGameState");
const glm::vec4 DebugConsoleGameState::SUCCESS_COLOR(0.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 DebugConsoleGameState::FAILURE_COLOR(1.0f, 0.0f, 0.0f, 1.0f);
const float DebugConsoleGameState::BIRDS_EYE_VIEW_CAMERA_LENSE_HEIGHT = 90.0f;
const int DebugConsoleGameState::SCROLL_LINE_THRESHOLD = 8;
const float DebugConsoleGameState::SCROLL_TOUCH_MIN_Y = 1.0f;
const float DebugConsoleGameState::SCROLL_MIN_Y = 1.5f;
const float DebugConsoleGameState::SCROLL_MAX_Y = 9.0f;

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::VInitialize()
{
    RegisterCommands();
    mSceneElementIds.clear();
    mPastCommandElementIds.clear();
    mCommandOutputElementIds.clear();
    mPastCommandHistoryIndex = -1;
    mLastEventType = 0;
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::DEBUG_CONSOLE_FONT_SHADER_FILE_NAME);
    
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
    // Overlay Alpha Update
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
    
    auto& inputContext = GameSingletons::GetInputContext();
    
    // Up/Down/Execute keys
    auto commandTextSoOpt = mScene->GetSceneObject(scene_object_constants::DEBUG_COMMAND_TEXT_SCENE_OBJECT_NAME);
    if (commandTextSoOpt)
    {
        auto& commandTextSo = commandTextSoOpt->get();
        
        if (inputContext.mEventType == SDL_KEYDOWN && mLastEventType != SDL_KEYDOWN)
        {
            if (inputContext.mKeyCode == SDL_SCANCODE_UP && mPastCommandElementIds.size() > 0)
            {
                mPastCommandHistoryIndex = (mPastCommandHistoryIndex + 1) % mPastCommandElementIds.size();
                GameSingletons::SetInputContextText(mScene->GetSceneObject(mPastCommandElementIds[mPastCommandElementIds.size() - 1 - mPastCommandHistoryIndex])->get().mText);
            }
            else if (inputContext.mKeyCode == SDL_SCANCODE_DOWN && mPastCommandElementIds.size() > 0)
            {
                mPastCommandHistoryIndex--;
                if (mPastCommandHistoryIndex < 0) mPastCommandHistoryIndex = static_cast<int>(mPastCommandElementIds.size()) - 1;
                GameSingletons::SetInputContextText(mScene->GetSceneObject(mPastCommandElementIds[mPastCommandElementIds.size() - 1 - mPastCommandHistoryIndex])->get().mText);
            }
            else if (inputContext.mKeyCode == SDL_SCANCODE_RETURN)
            {
                ExecuteCommand(inputContext.mText, commandTextSoOpt->get());
            }
        }
    
        commandTextSo.mText = GameSingletons::GetInputContext().mText;
    }
    
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& guiCamera = camOpt->get();
    
    // Back to game button press test
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
    
    // Output scrolling
    if (inputContext.mEventType == SDL_FINGERMOTION && mCommandOutputElementIds.size() > SCROLL_LINE_THRESHOLD)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        if (mPreviousMotionY > 0.0f && touchPos.y > SCROLL_TOUCH_MIN_Y)
        {
            float dy = touchPos.y - mPreviousMotionY;
            
            auto& firstLineSo = mScene->GetSceneObject(mCommandOutputElementIds.front())->get();
            auto& lastLineSo = mScene->GetSceneObject(mCommandOutputElementIds.back())->get();
            
            if (firstLineSo.mCustomPosition.y + dy < SCROLL_MAX_Y)
            {
                dy = SCROLL_MAX_Y - firstLineSo.mCustomPosition.y;
            }
            else if (lastLineSo.mCustomPosition.y + dy > SCROLL_MIN_Y)
            {
                dy = SCROLL_MIN_Y - lastLineSo.mCustomPosition.y;
            }
            
            for (const auto& elementId: mCommandOutputElementIds)
            {
                mScene->GetSceneObject(elementId)->get().mCustomPosition.y += dy;
            }
        }
        
        mPreviousMotionY = touchPos.y;
    }
    
    if (inputContext.mEventType == SDL_FINGERUP)
    {
        mPreviousMotionY = 0.0f;
    }
    
    // Screen exit
    if (completeButtonPressed || (inputContext.mKeyCode == SDL_SCANCODE_ESCAPE))
    {
        SDL_StopTextInput();
        GameSingletons::ConsumeInput();
        Complete();
    }
    
    mLastEventType = inputContext.mEventType;
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
    
    for (const auto& elementId: mCommandOutputElementIds)
    {
        mScene->RemoveAllSceneObjectsWithNameTag(elementId);
    }
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::RegisterCommands()
{
    mCommandMap[strutils::StringId("commands")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: commands");
        
        if (commandComponents.size() != 1)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        std::vector<std::string> output;
        for (const auto& entry: mCommandMap)
        {
            output.push_back(entry.first.GetString());
        }
        
        return CommandExecutionResult(true, output);
    };
    
    mCommandMap[strutils::StringId("physx")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: physx on|off");
        
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
    
    mCommandMap[strutils::StringId("bev")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: bev on|off");
        
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
    
    mCommandMap[strutils::StringId("getpos")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: getpos <scene_object_name>");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        if (!mScene->GetSceneObject(strutils::StringId(commandComponents[1])))
        {
            return CommandExecutionResult(false, "Scene Object not found");
        }
        
        auto& sceneObject = mScene->GetSceneObject(strutils::StringId(commandComponents[1]))->get();
        if (sceneObject.mBody)
        {
            return CommandExecutionResult(true, "Position: " + strutils::FloatToString(sceneObject.mBody->GetWorldCenter().x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mBody->GetWorldCenter().y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomPosition.z, 4));
        }
        else
        {
            return CommandExecutionResult(true, "Position: " + strutils::FloatToString(sceneObject.mCustomPosition.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomPosition.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomPosition.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("addpos")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: addpos <scene_object_name> dx dy dz");
        
        if (commandComponents.size() != 5)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        if (!mScene->GetSceneObject(strutils::StringId(commandComponents[1])))
        {
            return CommandExecutionResult(false, "Scene Object not found");
        }
        
        auto dx = std::stof(commandComponents[2]);
        auto dy = std::stof(commandComponents[3]);
        auto dz = std::stof(commandComponents[4]);
        
        auto& sceneObject = mScene->GetSceneObject(strutils::StringId(commandComponents[1]))->get();
        sceneObject.mCustomPosition.z += dz;
        
        if (sceneObject.mBody)
        {
            sceneObject.mBody->SetTransform(b2Vec2(sceneObject.mBody->GetWorldCenter().x + dx, sceneObject.mBody->GetWorldCenter().y + dy), 0.0f);
            return CommandExecutionResult(true, "New Position: " + strutils::FloatToString(sceneObject.mBody->GetWorldCenter().x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mBody->GetWorldCenter().y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomPosition.z, 4));
        }
        else
        {
            sceneObject.mCustomPosition.x += dx;
            sceneObject.mCustomPosition.y += dy;
            return CommandExecutionResult(true, "New Position: " + strutils::FloatToString(sceneObject.mCustomPosition.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomPosition.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomPosition.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("getscale")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: scale <scene_object_name>");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        if (!mScene->GetSceneObject(strutils::StringId(commandComponents[1])))
        {
            return CommandExecutionResult(false, "Scene Object not found");
        }
        
        auto& sceneObject = mScene->GetSceneObject(strutils::StringId(commandComponents[1]))->get();
        if (sceneObject.mBody)
        {
            return CommandExecutionResult(false, "Scene Object has a body!");
        }
        else
        {
            return CommandExecutionResult(true, "Scale: " + strutils::FloatToString(sceneObject.mCustomScale.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomScale.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomScale.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("addscale")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: addscale <scene_object_name> dsx dsy dsz");
        
        if (commandComponents.size() != 5)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        if (!mScene->GetSceneObject(strutils::StringId(commandComponents[1])))
        {
            return CommandExecutionResult(false, "Scene Object not found");
        }
        
        auto dsx = std::stof(commandComponents[2]);
        auto dsy = std::stof(commandComponents[3]);
        auto dsz = std::stof(commandComponents[4]);
        
        auto& sceneObject = mScene->GetSceneObject(strutils::StringId(commandComponents[1]))->get();
        if (sceneObject.mBody)
        {
            return CommandExecutionResult(false, "Scene Object has a body!");
        }
        else
        {
            sceneObject.mCustomScale.x += dsx;
            sceneObject.mCustomScale.y += dsy;
            sceneObject.mCustomScale.z += dsz;
            return CommandExecutionResult(true, "New Scale: " + strutils::FloatToString(sceneObject.mCustomScale.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomScale.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mCustomScale.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("visible_bodies")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: visible_bodies");
        
        if (commandComponents.size() != 1)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        std::vector<std::string> output;
        
        const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
        if (camOpt)
        {
            auto& cam = camOpt->get();
            for (const auto& so: mScene->GetSceneObjects())
            {
                if (so.mBody)
                {
                    const auto& bodyPosition = so.mBody->GetWorldCenter();
                    if (bodyPosition.x > -cam.GetCameraLenseWidth()/2 &&
                        bodyPosition.x < +cam.GetCameraLenseWidth()/2 &&
                        bodyPosition.y > -cam.GetCameraLenseHeight()/2 &&
                        bodyPosition.y < +cam.GetCameraLenseHeight()/2)
                    {
                        output.emplace_back(so.mNameTag.GetString() + " at " + strutils::FloatToString(bodyPosition.x, 4) + ", " + strutils::FloatToString(bodyPosition.y, 4));
                    }
                }
            }
        }
        return CommandExecutionResult(true, output);
    };
    
    mCommandMap[strutils::StringId("scene_objects")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: scene_objects");
        
        if (commandComponents.size() != 1)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        std::vector<std::string> output;
        for (const auto& so: mScene->GetSceneObjects())
        {
            if (so.mBody)
            {
                output.emplace_back(so.mNameTag.GetString() + " at " + strutils::FloatToString(so.mBody->GetWorldCenter().x, 4) + ", " + strutils::FloatToString(so.mBody->GetWorldCenter().y, 4));
            }
            else
            {
                output.emplace_back(so.mNameTag.GetString() + " at " + strutils::FloatToString(so.mCustomPosition.x, 4) + ", " + strutils::FloatToString(so.mCustomPosition.y, 4));
            }
        }
        
        return CommandExecutionResult(true, output);
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
    for (const auto& elementId: mCommandOutputElementIds)
    {
        mScene->RemoveAllSceneObjectsWithNameTag(elementId);
    }
    mCommandOutputElementIds.clear();
    
    auto commandOutputSoOpt = mScene->GetSceneObject(scene_object_constants::DEBUG_COMMAND_OUTPUT_SCENE_OBJECT_NAME);
    if (commandOutputSoOpt)
    {
        auto& commandOutputSo = commandOutputSoOpt->get();
        for (size_t i = 0; i < executionResult.mOutputMessage.size(); ++i)
        {
            SceneObject outputLineSceneObject;
            outputLineSceneObject.mNameTag = strutils::StringId(scene_object_constants::DEBUG_COMMAND_OUTPUT_LINE_NAME_PREFIX.GetString() +  std::to_string(i));
            outputLineSceneObject.mCustomPosition = commandOutputSo.mCustomPosition;
            outputLineSceneObject.mCustomPosition.y -= i * 1.0f;
            outputLineSceneObject.mCustomScale = commandOutputSo.mCustomScale;
            outputLineSceneObject.mText = executionResult.mOutputMessage[i];
            outputLineSceneObject.mFontName = commandOutputSo.mFontName;
            outputLineSceneObject.mMeshResourceId = commandOutputSo.mMeshResourceId;
            outputLineSceneObject.mShaderResourceId = resources::ResourceLoadingService::GetInstance().GetResourceIdFromPath(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::DEBUG_CONSOLE_FONT_SHADER_FILE_NAME);
            outputLineSceneObject.mShaderFloatVec4UniformValues[scene_object_constants::CUSTOM_COLOR_UNIFORM_NAME] = executionResult.mSuccess ? SUCCESS_COLOR : FAILURE_COLOR;
            outputLineSceneObject.mAnimation = commandOutputSoOpt->get().mAnimation->VClone();
            outputLineSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
            mCommandOutputElementIds.push_back(outputLineSceneObject.mNameTag);
            mScene->AddSceneObject(std::move(outputLineSceneObject));
        }
    }
}

///------------------------------------------------------------------------------------------------

void DebugConsoleGameState::PostCommandExecution(const std::string& command, const SceneObject& commandTextSo)
{
    // Create a past command SO out of current executed command
    {
        SceneObject pastTextSceneObject;
        pastTextSceneObject.mNameTag = strutils::StringId(scene_object_constants::DEBUG_PAST_COMMAND_LINE_NAME_PREFIX.GetString() +  std::to_string(mPastCommandElementIds.size()));
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
