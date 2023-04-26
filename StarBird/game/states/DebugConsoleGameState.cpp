///------------------------------------------------------------------------------------------------
///  DebugConsoleGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "DebugConsoleGameState.h"
#include "../GameConstants.h"
#include "../GameSingletons.h"
#include "../LevelUpdater.h"
#include "../PhysicsConstants.h"
#include "../Scene.h"
#include "../SceneObject.h"
#include "../SceneObjectUtils.h"
#include "../datarepos/FontRepository.h"
#include "../dataloaders/GUISceneLoader.h"
#include "../../resloading/ResourceLoadingService.h"

///------------------------------------------------------------------------------------------------

const strutils::StringId DebugConsoleGameState::STATE_NAME("DebugConsoleGameState");

static const glm::vec4 SUCCESS_COLOR(0.0f, 1.0f, 0.0f, 1.0f);
static const glm::vec4 FAILURE_COLOR(1.0f, 0.0f, 0.0f, 1.0f);

static const int SCROLL_LINE_THRESHOLD = 8;

static const float BIRDS_EYE_VIEW_CAMERA_LENSE_HEIGHT = 90.0f;
static const float SCROLL_TOUCH_MIN_Y = 1.0f;
static const float SCROLL_MIN_Y = 1.5f;
static const float SCROLL_MAX_Y = 9.0f;
static const float DEBUG_PAST_COMMAND_X_OFFSET = -1.0f;
static const float DEBUG_PAST_COMMAND_Y_OFFSET = 1.0f;

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
    resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::DEBUG_CONSOLE_FONT_SHADER_FILE_NAME);
    resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME);
    
    mScene->AddOverlayController(game_constants::FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED, game_constants::FULL_SCREEN_OVERLAY_MENU_MAX_ALPHA, true);
    
    GUISceneLoader loader;
    const auto& sceneDefinition = loader.LoadGUIScene("debug_console");
    
    for (const auto& guiElement: sceneDefinition.mGUIElements)
    {
        SceneObject guiSceneObject;
        guiSceneObject.mName = guiElement.mSceneObjectName;
        guiSceneObject.mPosition = guiElement.mPosition;
        guiSceneObject.mScale = guiElement.mScale;
        guiSceneObject.mText = guiElement.mText;
        guiSceneObject.mFontName = guiElement.mFontName;
        guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(guiElement.mTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), guiElement.mShaderResourceId, glm::vec3(1.0f), false);
        guiSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
        
        if (guiSceneObject.mFontName != strutils::StringId())
        {
            guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(guiSceneObject.mFontName)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), guiElement.mShaderResourceId, glm::vec3(1.0f), false);
        }
        
        mSceneElementIds.push_back(guiSceneObject.mName);
        mScene->AddSceneObject(std::move(guiSceneObject));
    }
    
    GameSingletons::SetInputContextText("");
    SDL_StartTextInput();
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective DebugConsoleGameState::VUpdate(const float dtMillis)
{
    auto& inputContext = GameSingletons::GetInputContext();
    
    // Up/Down/Execute keys
    auto commandTextSoOpt = mScene->GetSceneObject(game_constants::DEBUG_COMMAND_TEXT_SCENE_OBJECT_NAME);
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
                if (mPastCommandHistoryIndex < 0) mPastCommandHistoryIndex = static_cast<int>(mPastCommandElementIds.size()) - 1;
                GameSingletons::SetInputContextText(mScene->GetSceneObject(mPastCommandElementIds[mPastCommandElementIds.size() - 1 - mPastCommandHistoryIndex])->get().mText);
                mPastCommandHistoryIndex--;
            }
            else if (inputContext.mKeyCode == SDL_SCANCODE_RETURN)
            {
                ExecuteCommand(inputContext.mText, commandTextSoOpt->get());
                mPastCommandHistoryIndex = static_cast<int>(mPastCommandElementIds.size() - 1);
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
        
        auto continueButtonSoOpt = mScene->GetSceneObject(game_constants::DEBUG_BACK_TO_GAME_SCENE_OBJECT_NAME);
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
            
            if (firstLineSo.mPosition.y + dy < SCROLL_MAX_Y)
            {
                dy = SCROLL_MAX_Y - firstLineSo.mPosition.y;
            }
            else if (lastLineSo.mPosition.y + dy > SCROLL_MIN_Y)
            {
                dy = SCROLL_MIN_Y - lastLineSo.mPosition.y;
            }
            
            for (const auto& elementId: mCommandOutputElementIds)
            {
                mScene->GetSceneObject(elementId)->get().mPosition.y += dy;
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
        mScene->ResumeOverlayController();
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
        mScene->RemoveAllSceneObjectsWithName(elementId);
    }
    
    for (const auto& elementId: mPastCommandElementIds)
    {
        mScene->RemoveAllSceneObjectsWithName(elementId);
    }
    
    for (const auto& elementId: mCommandOutputElementIds)
    {
        mScene->RemoveAllSceneObjectsWithName(elementId);
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
    
    mCommandMap[strutils::StringId("god_mode")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: god_mode on|off");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        else if (commandComponents[1] != "on" && commandComponents[1] != "off")
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        GameSingletons::SetGodMode(commandComponents[1] == "on");
        return CommandExecutionResult(true, "God Mode turned " + commandComponents[1]);
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
        
        mScene->RemoveAllSceneObjectsWithName(game_constants::WALL_SCENE_OBJECT_NAME);
        
        if (commandComponents[1] == "on")
        {
            mPreviousCameraLenseHeight = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get().GetCameraLenseHeight();
            
            GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera(BIRDS_EYE_VIEW_CAMERA_LENSE_HEIGHT));
            
            if (mLevelUpdater)
            {
                mLevelUpdater->CreateLevelWalls(GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject)->get(), false);
            }
        }
        else if (mPreviousCameraLenseHeight > 0.0f)
        {
            GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera(mPreviousCameraLenseHeight));
            
            if (mLevelUpdater)
            {
                mLevelUpdater->CreateLevelWalls(GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get(), true);
            }
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
                                                               strutils::FloatToString(sceneObject.mPosition.z, 4));
        }
        else
        {
            return CommandExecutionResult(true, "Position: " + strutils::FloatToString(sceneObject.mPosition.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mPosition.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mPosition.z, 4));
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
        sceneObject.mPosition.z += dz;
        
        if (sceneObject.mBody)
        {
            sceneObject.mBody->SetTransform(b2Vec2(sceneObject.mBody->GetWorldCenter().x + dx, sceneObject.mBody->GetWorldCenter().y + dy), 0.0f);
            return CommandExecutionResult(true, "New Position: " + strutils::FloatToString(sceneObject.mBody->GetWorldCenter().x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mBody->GetWorldCenter().y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mPosition.z, 4));
        }
        else
        {
            sceneObject.mPosition.x += dx;
            sceneObject.mPosition.y += dy;
            return CommandExecutionResult(true, "New Position: " + strutils::FloatToString(sceneObject.mPosition.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mPosition.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mPosition.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("getscale")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: getscale <scene_object_name>");
        
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
            return CommandExecutionResult(true, "Scale: " + strutils::FloatToString(sceneObject.mScale.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mScale.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mScale.z, 4));
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
            sceneObject.mScale.x += dsx;
            sceneObject.mScale.y += dsy;
            sceneObject.mScale.z += dsz;
            return CommandExecutionResult(true, "New Scale: " + strutils::FloatToString(sceneObject.mScale.x, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mScale.y, 4) + ", " +
                                                               strutils::FloatToString(sceneObject.mScale.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("getrot")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: getrot <scene_object_name>");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        if (!mScene->GetSceneObject(strutils::StringId(commandComponents[1])))
        {
            return CommandExecutionResult(false, "Scene Object not found");
        }
        
        auto& sceneObject = mScene->GetSceneObject(strutils::StringId(commandComponents[1]))->get();
        
        return CommandExecutionResult(true, "Rotation: " + strutils::FloatToString(sceneObject.mRotation.x, 4) + ", " +
                                                           strutils::FloatToString(sceneObject.mRotation.y, 4) + ", " +
                                                           strutils::FloatToString(sceneObject.mRotation.z, 4));
    
    };
    
    mCommandMap[strutils::StringId("addrot")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: addrot <scene_object_name> drx dry drz");
        
        if (commandComponents.size() != 5)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        if (!mScene->GetSceneObject(strutils::StringId(commandComponents[1])))
        {
            return CommandExecutionResult(false, "Scene Object not found");
        }
        
        auto drx = std::stof(commandComponents[2]);
        auto dry = std::stof(commandComponents[3]);
        auto drz = std::stof(commandComponents[4]);
        
        auto& sceneObject = mScene->GetSceneObject(strutils::StringId(commandComponents[1]))->get();
        if (sceneObject.mBody)
        {
            return CommandExecutionResult(false, "Scene Object has a body!");
        }
        else
        {
            sceneObject.mRotation.x += drx;
            sceneObject.mRotation.y += dry;
            sceneObject.mRotation.z += drz;
            return CommandExecutionResult(true, "New Rotation: " + strutils::FloatToString(sceneObject.mRotation.x, 4) + ", " +
                                                                   strutils::FloatToString(sceneObject.mRotation.y, 4) + ", " +
                                                                   strutils::FloatToString(sceneObject.mRotation.z, 4));
        }
    };
    
    mCommandMap[strutils::StringId("game_speed")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: game_speed [<speed>]");
        
        if (commandComponents.size() != 1 && commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        
        if (commandComponents.size() == 2)
        {
            GameSingletons::SetGameSpeedMultiplier(std::stof(commandComponents[1]));
        }
        
        return CommandExecutionResult(true, "Game speed: " + std::to_string(GameSingletons::GetGameSpeedMultiplier()));
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
                        output.emplace_back(so.mName.GetString() + " at " + strutils::FloatToString(bodyPosition.x, 4) + ", " + strutils::FloatToString(bodyPosition.y, 4));
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
                output.emplace_back(so.mName.GetString() + " at " + strutils::FloatToString(so.mBody->GetWorldCenter().x, 4) + ", " + strutils::FloatToString(so.mBody->GetWorldCenter().y, 4));
            }
            else
            {
                output.emplace_back(so.mName.GetString() + " at " + strutils::FloatToString(so.mPosition.x, 4) + ", " + strutils::FloatToString(so.mPosition.y, 4) + ", "  + strutils::FloatToString(so.mPosition.z, 4));
            }
        }
        
        return CommandExecutionResult(true, output);
    };
    
    mCommandMap[strutils::StringId("scene_edit")] = [&](const std::vector<std::string>& commandComponents)
    {
        static const std::string USAGE_TEXT("Usage: scene_edit on|off");
        
        if (commandComponents.size() != 2)
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        else if (commandComponents[1] != "on" && commandComponents[1] != "off")
        {
            return CommandExecutionResult(false, USAGE_TEXT);
        }
        
        mScene->SetSceneEditMode(commandComponents[1] == "on");
        return CommandExecutionResult(true, "Scene edit turned " + commandComponents[1]);
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
        mScene->RemoveAllSceneObjectsWithName(elementId);
    }
    mCommandOutputElementIds.clear();
    
    auto commandOutputSoOpt = mScene->GetSceneObject(game_constants::DEBUG_COMMAND_OUTPUT_SCENE_OBJECT_NAME);
    if (commandOutputSoOpt)
    {
        auto& commandOutputSo = commandOutputSoOpt->get();
        for (size_t i = 0; i < executionResult.mOutputMessage.size(); ++i)
        {
            SceneObject outputLineSceneObject;
            outputLineSceneObject.mName = strutils::StringId(game_constants::DEBUG_COMMAND_OUTPUT_LINE_NAME_PREFIX.GetString() +  std::to_string(i));
            outputLineSceneObject.mPosition = commandOutputSo.mPosition;
            outputLineSceneObject.mPosition.y -= i * 1.0f;
            outputLineSceneObject.mScale = commandOutputSo.mScale;
            outputLineSceneObject.mText = executionResult.mOutputMessage[i];
            outputLineSceneObject.mFontName = commandOutputSo.mFontName;
            outputLineSceneObject.mShaderFloatVec4UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = executionResult.mSuccess ? SUCCESS_COLOR : FAILURE_COLOR;
            outputLineSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(commandOutputSo.mAnimation->VGetCurrentTextureResourceId(), commandOutputSo.mAnimation->VGetCurrentMeshResourceId(), resources::ResourceLoadingService::GetInstance().GetResourceIdFromPath(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::DEBUG_CONSOLE_FONT_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            outputLineSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
            mCommandOutputElementIds.push_back(outputLineSceneObject.mName);
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
        pastTextSceneObject.mName = strutils::StringId(game_constants::DEBUG_PAST_COMMAND_LINE_NAME_PREFIX.GetString() +  std::to_string(mPastCommandElementIds.size()));
        pastTextSceneObject.mPosition = commandTextSo.mPosition;
        pastTextSceneObject.mPosition.x += DEBUG_PAST_COMMAND_X_OFFSET;
        pastTextSceneObject.mScale = commandTextSo.mScale;
        pastTextSceneObject.mText = commandTextSo.mText;
        pastTextSceneObject.mFontName = commandTextSo.mFontName;
        pastTextSceneObject.mAnimation = commandTextSo.mAnimation->VClone();
        pastTextSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
        mPastCommandElementIds.push_back(pastTextSceneObject.mName);
        mScene->AddSceneObject(std::move(pastTextSceneObject));
    }
    
    // Push all past commands up
    for (const auto& id: mPastCommandElementIds)
    {
        auto soOpt = mScene->GetSceneObject(id);
        if (soOpt)
        {
            soOpt->get().mPosition.y += DEBUG_PAST_COMMAND_Y_OFFSET;
        }
    }
    
    GameSingletons::ConsumeInput();
    GameSingletons::SetInputContextText("");
}

///------------------------------------------------------------------------------------------------
