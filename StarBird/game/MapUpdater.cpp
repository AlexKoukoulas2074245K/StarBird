///------------------------------------------------------------------------------------------------
///  MapUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "MapUpdater.h"
#include "GameObjectConstants.h"
#include "GameSingletons.h"
#include "SceneObjectConstants.h"
#include "Scene.h"
#include "states/DebugConsoleGameState.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

///------------------------------------------------------------------------------------------------

MapUpdater::MapUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_object_constants::BACKGROUND_SCALE;
        bgSO.mScale.x *= 2.0f;
        bgSO.mScale.y *= 2.0f;
        bgSO.mPosition.z = game_object_constants::BACKGROUND_Z;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mName = scene_object_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    const auto segments = 20;
    const auto scaleFactor = 2.0f;
    const auto block = (math::PI * 4)/segments;
    for (int i = 0; i < segments; ++i)
    {
        SceneObject starSO;
        
        starSO.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + (math::RandomSign() == 1 ? "octo_star.bmp" : "quad_star.bmp")), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(0.5f),  game_object_constants::PLAYER_PULSE_SHIELD_ANIM_SPEED/5.0f, game_object_constants::PLAYER_PULSE_SHIELD_ENLARGEMENT_FACTOR/10.0f, false);
        starSO.mSceneObjectType = SceneObjectType::GUIObject;
        starSO.mName = strutils::StringId("star_" + std::to_string(i));
        starSO.mScale.x = 1.0f;
        starSO.mScale.y = 1.0f;
        
        
        starSO.mPosition.x = -math::Cosf(i * block) * (segments - i)/((float)segments) * scaleFactor * math::RandomFloat(4.0f, 6.0);
        starSO.mPosition.y = math::Sinf(i * block) * (segments - i)/((float)segments) * scaleFactor * math::RandomFloat(4.0f, 6.0);
        
        Log(LogType::INFO, "%.6f, %.6f", starSO.mPosition.x, starSO.mPosition.y);
        starSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(starSO));
    }
    
    for (int i = 0; i < segments - 1; ++i)
    {
        auto currentStarOpt = mScene.GetSceneObject(strutils::StringId("star_" + std::to_string(i)));
        auto nextStarOpt = mScene.GetSceneObject(strutils::StringId("star_" + std::to_string(i + 1)));
        
        if (currentStarOpt && nextStarOpt)
        {
            glm::vec3 dirToNext = nextStarOpt->get().mPosition - currentStarOpt->get().mPosition;
            
            SceneObject pathSO;
            pathSO.mScale = glm::vec3(glm::length(dirToNext), glm::length(dirToNext), 1.0f);
            
            pathSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "star_path.bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), pathSO.mScale, false);
            pathSO.mSceneObjectType = SceneObjectType::GUIObject;
            pathSO.mName = strutils::StringId("star_" + std::to_string(i));
            
            pathSO.mPosition.x = (currentStarOpt->get().mPosition.x + nextStarOpt->get().mPosition.x)/2.0f;
            pathSO.mPosition.y = (currentStarOpt->get().mPosition.y + nextStarOpt->get().mPosition.y)/2.0f;
            
            pathSO.mRotation.z = -math::Arctan2(dirToNext.x, dirToNext.y) + math::PI;
            
            pathSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
            mScene.AddSceneObject(std::move(pathSO));
        }
    }
    
    auto& guiCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject)->get();
    
    glm::vec3 position;
    position.x = scaleFactor * -math::Cosf(0) * 5.0f;
    position.y = scaleFactor * math::Sinf(0) * 5.0f;
    position.z = 0.0f;
    guiCamera.SetPosition(position);
}

///------------------------------------------------------------------------------------------------

void MapUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    static glm::vec3 previousMotion;
    
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::CONTINUE)
    {
        
        auto& inputContext = GameSingletons::GetInputContext();
        auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
        auto& guiCamera = camOpt->get();
        
        if (inputContext.mEventType == SDL_FINGERMOTION)
        {
            auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
            
            if (glm::length(previousMotion) > 0.0f)
            {
                const auto deltaMotion = -(touchPos - previousMotion)*0.8f;
                guiCamera.SetPosition(guiCamera.GetPosition() + deltaMotion);
            }
            
            previousMotion = touchPos;
        }
        
        if (inputContext.mEventType == SDL_FINGERUP)
        {
            previousMotion = glm::vec3(0.0f);
        }
        
        for (auto& sceneObject: sceneObjects)
        {
            if (sceneObject.mAnimation)
            {
                sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void MapUpdater::OnAppStateChange(Uint32 event)
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
                OpenDebugConsole();
            }
#endif
        } break;
    }
}

///------------------------------------------------------------------------------------------------

std::string MapUpdater::GetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void MapUpdater::OpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------
