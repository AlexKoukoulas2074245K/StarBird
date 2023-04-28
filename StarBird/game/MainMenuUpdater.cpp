///------------------------------------------------------------------------------------------------
///  MainMenuUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "MainMenuUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "states/DebugConsoleGameState.h"
#include "dataloaders/GUISceneLoader.h"
#include "datarepos/FontRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

static const strutils::StringId PLAY_TEXT_SCENE_OBJECT_NAME = strutils::StringId("play_button");
static const strutils::StringId CONTINUE_TEXT_SCENE_OBJECT_NAME = strutils::StringId("continue_button");
static const strutils::StringId NEW_GAME_TEXT_SCENE_OBJECT_NAME = strutils::StringId("new_game_button");
static const strutils::StringId SEED_VALUE_SCENE_OBJECT_NAME = strutils::StringId("current_seed_value");

///------------------------------------------------------------------------------------------------

MainMenuUpdater::MainMenuUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mTransitioning(false)
    
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    CreateSceneObjects();
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective MainMenuUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mTransitioning)
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return PostStateUpdateDirective::BLOCK_UPDATE;
    
    auto& inputContext = GameSingletons::GetInputContext();
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    auto& guiCamera = camOpt->get();
    
    // Touch Position
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        auto playSoOpt = mScene.GetSceneObject(PLAY_TEXT_SCENE_OBJECT_NAME);
        if (playSoOpt && scene_object_utils::IsPointInsideSceneObject(*playSoOpt, touchPos))
        {
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            mTransitioning = true;
        }
    }
    
    UpdateBackground(dtMillis);
    
    // Animate all SOs
    for (auto& sceneObject: sceneObjects)
    {
        if (sceneObject.mAnimation && !sceneObject.mAnimation->VIsPaused())
        {
            sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
        }
        
        for (auto& extraAnimation: sceneObject.mExtraCompoundingAnimations)
        {
            if (!extraAnimation->VIsPaused())
            {
                extraAnimation->VUpdate(dtMillis, sceneObject);
            }
        }
    }

    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void MainMenuUpdater::VOnAppStateChange(Uint32 event)
{
    static bool hasLeftForegroundOnce = false;
    
    switch (event)
    {
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND:
        {
#ifdef DEBUG
            hasLeftForegroundOnce = true;
#endif
        } break;
            
        case SDL_APP_WILLENTERFOREGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
        {
#ifdef DEBUG
            if (hasLeftForegroundOnce)
            {
                VOpenDebugConsole();
            }
#endif
        } break;
    }
}

///------------------------------------------------------------------------------------------------

std::string MainMenuUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId MainMenuUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void MainMenuUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void MainMenuUpdater::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_constants::BACKGROUND_SCALE;
        bgSO.mPosition.z = game_constants::BACKGROUND_Z;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::DEFAULT_BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::TEXTURE_OFFSET_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // GUI Scene Elements
    {
        GUISceneLoader loader;
        const auto& sceneDefinition = loader.LoadGUIScene("main_menu_scene");
        
        for (const auto& guiElement: sceneDefinition.mGUIElements)
        {
            SceneObject guiSceneObject;
            guiSceneObject.mName = guiElement.mSceneObjectName;
            guiSceneObject.mPosition = guiElement.mPosition;
            guiSceneObject.mScale = guiElement.mScale;
            guiSceneObject.mText = guiElement.mText;
            guiSceneObject.mFontName = guiElement.mFontName;
            guiSceneObject.mInvisible = guiElement.mInvisible;
            
            guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(guiElement.mTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), guiElement.mShaderResourceId, glm::vec3(1.0f), false);
            guiSceneObject.mSceneObjectType = SceneObjectType::GUIObject;
            
            if (guiSceneObject.mFontName != strutils::StringId())
            {
                guiSceneObject.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(guiSceneObject.mFontName)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), guiElement.mShaderResourceId, glm::vec3(1.0f), false);
            }
            
            mScene.AddSceneObject(std::move(guiSceneObject));
        }
        
        auto seedValueSoOpt = mScene.GetSceneObject(SEED_VALUE_SCENE_OBJECT_NAME);
        if (seedValueSoOpt)
        {
            seedValueSoOpt->get().mText = std::to_string(GameSingletons::GetMapGenerationSeed());
        }
        
        auto playButtonSoOpt = mScene.GetSceneObject(PLAY_TEXT_SCENE_OBJECT_NAME);
        auto continueButtonSoOpt = mScene.GetSceneObject(CONTINUE_TEXT_SCENE_OBJECT_NAME);
        
        // Detect new game
        if (GameSingletons::GetMapLevel() == 0 && GameSingletons::GetCurrentMapCoord() == MapCoord(game_constants::DEFAULT_MAP_COORD_COL, game_constants::DEFAULT_MAP_COORD_ROW))
        {
            if (playButtonSoOpt)
            {
                playButtonSoOpt->get().mInvisible = false;
            }
        }
        else
        {
            if (continueButtonSoOpt)
            {
                continueButtonSoOpt->get().mInvisible = false;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void MainMenuUpdater::UpdateBackground(const float dtMillis)
{
    static float msAccum = 0.0f;
    msAccum += dtMillis * game_constants::BACKGROUND_SPEED;
    msAccum = std::fmod(msAccum, 1.0f);
    
    auto bgSO = mScene.GetSceneObject(game_constants::BACKGROUND_SCENE_OBJECT_NAME);
    if (bgSO)
    {
       bgSO->get().mShaderFloatUniformValues[game_constants::GENERIC_TEXTURE_OFFSET_UNIFORM_NAME] = -msAccum;
    }
}

///------------------------------------------------------------------------------------------------
