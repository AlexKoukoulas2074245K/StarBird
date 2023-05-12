///------------------------------------------------------------------------------------------------
///  MapUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "FullScreenOverlayController.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "LevelGeneration.h"
#include "MapUpdater.h"
#include "PersistenceUtils.h"
#include "ObjectiveCUtils.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "Sounds.h"
#include "states/DebugConsoleGameState.h"
#include "states/SettingsMenuGameState.h"
#include "datarepos/FontRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../utils/Logging.h"
#include "../utils/ObjectiveCUtils.h"

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

static const std::unordered_map<Map::NodeType, Scene::SceneType> NODE_TYPE_TO_SCENE_TYPE =
{
    { Map::NodeType::NORMAL_ENCOUNTER, Scene::SceneType::LEVEL },
    { Map::NodeType::HARD_ENCOUNTER, Scene::SceneType::LEVEL },
    { Map::NodeType::BOSS_ENCOUNTER, Scene::SceneType::LEVEL },
    { Map::NodeType::LAB, Scene::SceneType::LAB },
    { Map::NodeType::EVENT, Scene::SceneType::LAB },
};

static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT");

static const char* MAP_PATH_NAME_SUFFIX = "_PATH";
static const char* CONFIRMATION_BUTTON_TEXTURE_FILE_NAME = "confirmation_button_mm.bmp";

static const glm::vec3 CONFIRMATION_BUTTON_POSITION = glm::vec3(0.0f, -8.0f, 0.0f);
static const glm::vec3 CONFIRMATION_BUTTON_SCALE = glm::vec3(3.5f, 3.5f, 0.0f);

static const glm::vec3 CONFIRMATION_BUTTON_TEXT_POSITION = glm::vec3(-0.49f, -8.27f, 0.5f);
static const glm::vec3 CONFIRMATION_BUTTON_TEXT_SCALE = glm::vec3(0.007f, 0.007f, 1.0f);

static const float CONFIRMATION_BUTTON_ROTATION_SPEED = 0.0002f;
static const float CONFIRMATION_BUTTON_PULSING_SPEED = 0.02f;
static const float CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR = 1.0f/10.0f;
static const float CONFIRMATION_BUTTON_TEXT_PULSING_ENLARGEMENT_FACTOR = 1.0f/4000.0f;

static const float MAX_MAP_VELOCITY_LENGTH = 5.0f;
static const float MAP_VELOCITY_DAMPING = 0.9f;
static const float MAP_VELOCITY_INTEGRATION_SPEED = 0.04f;
static const float CAMERA_MAX_ZOOM_FACTOR = 2.4f;
static const float CAMERA_INIT_ZOOM_FACTOR = 0.9f;
static const float CAMERA_MIN_ZOOM_FACTOR = 0.4f;
static const float CAMERA_ZOOM_SPEED = 0.1f;
static const float MIN_CAMERA_VELOCITY_TO_START_MOVEMENT = 0.0001f;

///------------------------------------------------------------------------------------------------

MapUpdater::MapUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mMap(scene, GameSingletons::GetMapGenerationSeed(), glm::ivec2(9, 5), GameSingletons::GetCurrentMapCoord(), true)
    , mSelectedMapCoord(MapCoord(0, 0))
    , mCurrentMapCoord(GameSingletons::GetCurrentMapCoord())
    , mLastInputContextEventType(0)
    , mTransitioning(false)
    
{
    persistence_utils::BuildProgressSaveFile();
    
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    mStateMachine.RegisterState<SettingsMenuGameState>();
    
    auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
    
    // Center camera to the midpoint of all available nodes
    glm::vec3 positionAccum = mMap.GetMapData().at(GameSingletons::GetCurrentMapCoord()).mPosition;
    const auto& activeCoords = mMap.GetMapData().at(GameSingletons::GetCurrentMapCoord()).mNodeLinks;
    for (auto& linkedCoord: activeCoords)
    {
        positionAccum += mMap.GetMapData().at(linkedCoord).mPosition;
    }
    
    worldCamera.SetPosition(glm::vec3(positionAccum.x/(activeCoords.size() + 1), positionAccum.y/(activeCoords.size() + 1), 0.0f));
    worldCamera.SetZoomFactor(CAMERA_INIT_ZOOM_FACTOR);
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Recreate confirmation button
    SceneObject confirmationButtonSo;
    confirmationButtonSo.mPosition = CONFIRMATION_BUTTON_POSITION;
    confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
    confirmationButtonSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CONFIRMATION_BUTTON_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Z, 0.0f, CONFIRMATION_BUTTON_ROTATION_SPEED, false);
    confirmationButtonSo.mSceneObjectType = SceneObjectType::GUIObject;
    confirmationButtonSo.mName = CONFIRMATION_BUTTON_NAME;
    confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    confirmationButtonSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
    mScene.AddSceneObject(std::move(confirmationButtonSo));
    
    // Confirmation button text
    SceneObject confirmationButtonTextSo;
    confirmationButtonTextSo.mPosition = CONFIRMATION_BUTTON_TEXT_POSITION;
    confirmationButtonTextSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
    confirmationButtonTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    confirmationButtonTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
    confirmationButtonTextSo.mSceneObjectType = SceneObjectType::GUIObject;
    confirmationButtonTextSo.mName = CONFIRMATION_BUTTON_TEXT_NAME;
    confirmationButtonTextSo.mText = "Visit";
    confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene.AddSceneObject(std::move(confirmationButtonTextSo));
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective MapUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mTransitioning)
    {
        auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
        auto confirmationButtonTextSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
        
        if (confirmationButtonSoOpt && confirmationButtonTextSoOpt)
        {
            auto& confirmationButtonSo = confirmationButtonSoOpt->get();
            auto& confirmationButtonTextSo = confirmationButtonTextSoOpt->get();
            
            confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
            {
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            }
            
            confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
            {
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            }
            
            confirmationButtonSo.mExtraCompoundingAnimations.front()->VUpdate(dtMillis, confirmationButtonSo);
            confirmationButtonTextSo.mExtraCompoundingAnimations.front()->VUpdate(dtMillis, confirmationButtonTextSo);
        }
        
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    static glm::vec3 originTouchPos;
    static glm::vec3 cameraVelocity;
    static float previousPinchDistance = 0.0f;
    
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return PostStateUpdateDirective::BLOCK_UPDATE;
    
    auto& inputContext = GameSingletons::GetInputContext();
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    
    auto camPos = worldCamera.GetPosition();
    auto camZoom = worldCamera.GetZoomFactor();
    
    if (mSelectedMapCoord != MapCoord(0, 0))
    {
        if (!mTransitioning)
        {
            const auto& guiCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject)->get();
            
            auto currentGuiTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
            
            if (inputContext.mEventType == SDL_FINGERDOWN && mLastInputContextEventType != SDL_FINGERDOWN)
            {
                auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
                if (confirmationButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(*confirmationButtonSoOpt, currentGuiTouchPos))
                {
                    OnConfirmationButtonPressed();
                    
                    GameSingletons::SetCurrentMapCoord(mSelectedMapCoord);
                    mTransitioning = true;
                    
                    auto selectedNodeType = mMap.GetMapData().at(mSelectedMapCoord).mNodeType;
                    
                    assert(NODE_TYPE_TO_SCENE_TYPE.contains(selectedNodeType));
                    auto nextSceneType = NODE_TYPE_TO_SCENE_TYPE.at(selectedNodeType);
                    
                    if (selectedNodeType == Map::NodeType::EVENT)
                    {
                        OnEventNodeSelected();
                    }
                    else
                    {
                        mScene.ChangeScene(Scene::TransitionParameters(nextSceneType, nextSceneType == Scene::SceneType::LEVEL ? (objectiveC_utils::BuildLocalFileSaveLocation(mSelectedMapCoord.ToString())) : "", true));
                    }
                    
                    objectiveC_utils::PlaySound(sounds::BUTTON_PRESS_SFX);
                }
                else
                {
                    OnLevelDeselection();
                    mSelectedMapCoord = MapCoord(0, 0);
                        
                    originTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
                    
                    if (CheckForActiveLevelSelection(originTouchPos))
                    {
                        OnLevelSelection();
                    }
                }
            }
        }
    }
    else
    {
        // Touch Position and Map velocity reset on FingerDown
        if (inputContext.mEventType == SDL_FINGERDOWN)
        {
            originTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
            cameraVelocity = glm::vec3(0.0f);
            
            if (mLastInputContextEventType != SDL_FINGERDOWN)
            {
                if (CheckForActiveLevelSelection(originTouchPos))
                {
                    OnLevelSelection();
                }
            }
        }
        // Map Position/Zoom flow on FingerMotion
        else if (inputContext.mEventType == SDL_FINGERMOTION)
        {
            // Pinch zoom flow
            if (GameSingletons::GetInputContext().mPinchDistance > 0.0f && previousPinchDistance > 0.0f && GameSingletons::GetInputContext().mMultiGestureActive)
            {
                camZoom = worldCamera.GetZoomFactor() + dtMillis * (GameSingletons::GetInputContext().mPinchDistance - previousPinchDistance) * CAMERA_ZOOM_SPEED;
            }
            // Pan flow
            else if (glm::length(originTouchPos) != 0.0f && GameSingletons::GetInputContext().mMultiGestureActive == false)
            {
                const auto deltaMotion = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix()) - originTouchPos;
                
                if (glm::length(deltaMotion) < MAX_MAP_VELOCITY_LENGTH)
                {
                    cameraVelocity = deltaMotion;
                }
            }
        }
        // Reset touch pos on FingerUp
        else if (inputContext.mEventType == SDL_FINGERUP)
        {
             originTouchPos = glm::vec3(0.0f);
        };
        
        // Calculate potential new camera position
        if (glm::length(cameraVelocity) > MIN_CAMERA_VELOCITY_TO_START_MOVEMENT)
        {
            camPos = worldCamera.GetPosition() - cameraVelocity * dtMillis * MAP_VELOCITY_INTEGRATION_SPEED *  1.0f/worldCamera.GetZoomFactor();
            cameraVelocity.x *= MAP_VELOCITY_DAMPING;
            cameraVelocity.y *= MAP_VELOCITY_DAMPING;
        }
        else
        {
            cameraVelocity = glm::vec3(0.0f);
        }
        
        // Clamp and apply camera position
        camPos.x = math::Max(math::Min(game_constants::MAP_MAX_WORLD_BOUNDS.x, camPos.x), game_constants::MAP_MIN_WORLD_BOUNDS.x);
        camPos.y = math::Max(math::Min(game_constants::MAP_MAX_WORLD_BOUNDS.y, camPos.y), game_constants::MAP_MIN_WORLD_BOUNDS.y);
        worldCamera.SetPosition(camPos);
        
        // Clamp and apply camera zoom
        camZoom = math::Max(math::Min(CAMERA_MAX_ZOOM_FACTOR, camZoom), CAMERA_MIN_ZOOM_FACTOR);
        worldCamera.SetZoomFactor(camZoom);
        
        // Keep track of previous finger pinch distance
        previousPinchDistance = GameSingletons::GetInputContext().mPinchDistance;
    }
    
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    auto confirmationButtonTextSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
    
    if (confirmationButtonSoOpt && confirmationButtonTextSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        auto& confirmationButtonTextSo = confirmationButtonTextSoOpt->get();
        
        if (mSelectedMapCoord != MapCoord(0, 0))
        {
            confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
            {
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            }
            
            confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
            {
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            }
        }
        else
        {
            confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
            {
                confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            }
            
            confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
            if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
            {
                confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            }
        }
    }
    
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

    mLastInputContextEventType = inputContext.mEventType;
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void MapUpdater::VOnAppStateChange(Uint32 event)
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

std::string MapUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId MapUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void MapUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void MapUpdater::VOpenSettingsMenu()
{
    mStateMachine.PushState(SettingsMenuGameState::STATE_NAME);
}

///------------------------------------------------------------------------------------------------

bool MapUpdater::CheckForActiveLevelSelection(const glm::vec3& touchPos)
{
    const auto& currentMapCoord = GameSingletons::GetCurrentMapCoord();
    for (const auto& linkedMapCoord: mMap.GetMapData().at(currentMapCoord).mNodeLinks)
    {
        if (scene_object_utils::IsPointInsideSceneObject(mScene.GetSceneObject(strutils::StringId(linkedMapCoord.ToString()))->get(), glm::vec2(touchPos.x, touchPos.y)))
        {
            mSelectedMapCoord = linkedMapCoord;
            objectiveC_utils::PlaySound(sounds::WHOOSH_SFX);
            return true;
        }
    }
    
    return false;
}

///------------------------------------------------------------------------------------------------

void MapUpdater::OnLevelSelection()
{
    const auto& mapData = mMap.GetMapData();
    const auto& previousMapNode = mapData.at(mCurrentMapCoord);
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    for (const auto& linkedCoord: previousMapNode.mNodeLinks)
    {
        if (linkedCoord == mSelectedMapCoord) continue;
        
        auto nodeSoOpt = mScene.GetSceneObject(strutils::StringId(linkedCoord.ToString()));
        auto nodeRingSoOpt = mScene.GetSceneObject(strutils::StringId("PLANET_RING_" + linkedCoord.ToString()));
        
        if (nodeSoOpt)
        {
            auto& nodeSo = nodeSoOpt->get();
            
            nodeSo.mAnimation->VPause();
            nodeSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::GRAYSCALE_SHADER_FILE_NAME));
            
            for (auto& extraAnimation: nodeSo.mExtraCompoundingAnimations)
            {
                extraAnimation->VPause();
            }
        }
        
        if (nodeRingSoOpt)
        {
            auto& nodeRingSo = nodeRingSoOpt->get();
            
            nodeRingSo.mAnimation->VPause();
            nodeRingSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::GRAYSCALE_SHADER_FILE_NAME));
            
            for (auto& extraAnimation: nodeRingSo.mExtraCompoundingAnimations)
            {
                extraAnimation->VPause();
            }
        }
        
        glm::vec3 dirToNext = mapData.at(linkedCoord).mPosition - previousMapNode.mPosition;
        auto pathSegments = 2 * static_cast<int>(glm::length(dirToNext));
        for (int i = 0; i < pathSegments; ++i)
        {
            auto pathSegmentName = strutils::StringId(mCurrentMapCoord.ToString() + "-" + linkedCoord.ToString() + "_" + std::to_string(i) + MAP_PATH_NAME_SUFFIX);
            auto pathSegmentSoOpt = mScene.GetSceneObject(pathSegmentName);
            if (pathSegmentSoOpt)
            {
                auto& pathSegmentSo = pathSegmentSoOpt->get();
                pathSegmentSo.mAnimation->VPause();
                pathSegmentSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::GRAYSCALE_SHADER_FILE_NAME));
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void MapUpdater::OnLevelDeselection()
{
    const auto& mapData = mMap.GetMapData();
    const auto& previousMapNode = mapData.at(mCurrentMapCoord);
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    for (const auto& linkedCoord: previousMapNode.mNodeLinks)
    {
        if (linkedCoord == mSelectedMapCoord) continue;
        
        auto nodeSoOpt = mScene.GetSceneObject(strutils::StringId(linkedCoord.ToString()));
        auto nodeRingSoOpt = mScene.GetSceneObject(strutils::StringId("PLANET_RING_" + linkedCoord.ToString()));
        
        if (nodeSoOpt)
        {
            auto& nodeSo = nodeSoOpt->get();
            
            nodeSo.mAnimation->VResume();
            
            switch (mapData.at(linkedCoord).mNodeType)
            {
                case Map::NodeType::NORMAL_ENCOUNTER:
                case Map::NodeType::HARD_ENCOUNTER:
                {
                    nodeSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::HUE_SHIFT_SHADER_FILE_NAME));
                } break;
                
                case Map::NodeType::LAB:
                case Map::NodeType::EVENT:
                {
                    nodeSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME));
                } break;
                    
                default: break;
            }
            
            
            for (auto& extraAnimation: nodeSo.mExtraCompoundingAnimations)
            {
                extraAnimation->VResume();
            }
        }
        
        if (nodeRingSoOpt)
        {
            auto& nodeRingSo = nodeRingSoOpt->get();
            
            nodeRingSo.mAnimation->VResume();
            nodeRingSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME));
            
            for (auto& extraAnimation: nodeRingSo.mExtraCompoundingAnimations)
            {
                extraAnimation->VResume();
            }
        }
        
        glm::vec3 dirToNext = mapData.at(linkedCoord).mPosition - previousMapNode.mPosition;
        auto pathSegments = 2 * static_cast<int>(glm::length(dirToNext));
        for (int i = 0; i < pathSegments; ++i)
        {
            auto pathSegmentName = strutils::StringId(mCurrentMapCoord.ToString() + "-" + linkedCoord.ToString() + "_" + std::to_string(i) + MAP_PATH_NAME_SUFFIX);
            auto pathSegmentSoOpt = mScene.GetSceneObject(pathSegmentName);
            if (pathSegmentSoOpt)
            {
                auto& pathSegmentSo = pathSegmentSoOpt->get();
                pathSegmentSo.mAnimation->ChangeShaderResourceId(resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME));
                pathSegmentSo.mAnimation->VResume();
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void MapUpdater::OnConfirmationButtonPressed()
{
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    auto confirmationButtonTexSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
    
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        confirmationButtonSo.mScale = CONFIRMATION_BUTTON_SCALE;
        
        confirmationButtonSo.mExtraCompoundingAnimations.clear();
        confirmationButtonSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONFIRMATION_BUTTON_PULSING_SPEED, CONFIRMATION_BUTTON_PULSING_ENLARGEMENT_FACTOR, false));
    }
    
    if (confirmationButtonTexSoOpt)
    {
        auto& confirmationButtonTextSo = confirmationButtonTexSoOpt->get();
        confirmationButtonTextSo.mScale = CONFIRMATION_BUTTON_TEXT_SCALE;
        
        confirmationButtonTextSo.mExtraCompoundingAnimations.clear();
        confirmationButtonTextSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonTextSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentShaderResourceId(), CONFIRMATION_BUTTON_TEXT_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, CONFIRMATION_BUTTON_PULSING_SPEED, CONFIRMATION_BUTTON_TEXT_PULSING_ENLARGEMENT_FACTOR, false));
    }
}

///------------------------------------------------------------------------------------------------

void MapUpdater::OnEventNodeSelected()
{
    static const math::ProbabilityDistribution probDist =
    {
        0.1f, // Lab
        0.1f, // Level
        0.8f, // Event
    };
    
    // Avoid same event for the whole map since the seed is set each time the map
    // gets created
    for (int i = 0; i < mCurrentMapCoord.mCol; ++i) math::ControlledRandomInt();
    
    auto selectedIndex = math::ControlledIndexSelectionFromDistribution(probDist);
    
    switch (selectedIndex)
    {
        case 0:
        {
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::LAB, "", true));
        } break;
            
        case 1:
        {
            level_generation::GenerateLevel(mSelectedMapCoord, mMap.GetMapData().at(mSelectedMapCoord));
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::LEVEL, objectiveC_utils::BuildLocalFileSaveLocation(mSelectedMapCoord.ToString()), true));
        } break;
            
        case 2:
        {
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::EVENT, "", true));
        }break;
    }
}

///------------------------------------------------------------------------------------------------
