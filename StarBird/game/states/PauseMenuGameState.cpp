///------------------------------------------------------------------------------------------------
///  PauseMenuGameState.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "PauseMenuGameState.h"
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

const strutils::StringId PauseMenuGameState::STATE_NAME("PauseMenuGameState");

///------------------------------------------------------------------------------------------------

void PauseMenuGameState::VInitialize()
{
    mSceneElementIds.clear();
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    mScene->AddOverlayController(game_constants::FULL_SCREEN_OVERLAY_MENU_DARKENING_SPEED, game_constants::FULL_SCREEN_OVERLAY_MENU_MAX_ALPHA, true);
    
    GUISceneLoader loader;
    const auto& sceneDefinition = loader.LoadGUIScene("pause_menu_scene");
    
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
        
        mSceneElementIds.push_back(guiSceneObject.mName);
        mScene->AddSceneObject(std::move(guiSceneObject));
    }
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective PauseMenuGameState::VUpdate(const float dtMillis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    const auto& guiCamera = camOpt->get();
    const auto& inputContext = GameSingletons::GetInputContext();
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        auto continueButtonSoOpt = mScene->GetSceneObject(strutils::StringId("continue_button"));
        if (continueButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(continueButtonSoOpt->get(), touchPos))
        {
            mScene->ResumeOverlayController();
            GameSingletons::ConsumeInput();
            Complete();
        }
    }
    
    return PostStateUpdateDirective::BLOCK_UPDATE;
}

///------------------------------------------------------------------------------------------------

void PauseMenuGameState::VDestroy()
{
    for (auto elementId: mSceneElementIds)
    {
        mScene->RemoveAllSceneObjectsWithName(elementId);
    }
}

///------------------------------------------------------------------------------------------------
