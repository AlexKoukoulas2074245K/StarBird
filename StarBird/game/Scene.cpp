///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "GameConstants.h"
#include "LabUpdater.h"
#include "MapUpdater.h"
#include "LevelUpdater.h"
#include "PhysicsConstants.h"
#include "SceneObjectUtils.h"
#include "StatsUpgradeUpdater.h"
#include "ObjectTypeDefinitionRepository.h"
#include "states/DebugConsoleGameState.h"
#include "dataloaders/LevelDataLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"

#include <algorithm>
#include <Box2D/Box2D.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SCENE_EDIT_RESULT_TEXT_NAME_1 = strutils::StringId("SCENE_EDIT_RESULT_TEXT_1");
static const strutils::StringId SCENE_EDIT_RESULT_TEXT_NAME_2 = strutils::StringId("SCENE_EDIT_RESULT_TEXT_2");

static const glm::vec3 GUI_CRYSTAL_COUNT_HOLDER_SCALE = glm::vec3(2.5f, 3.5f, 1.0f);
static const glm::vec3 GUI_CRYSTAL_COUNT_HOLDER_POSITION = glm::vec3(-4.2f, -10.9f, 2.0f);

static const glm::vec3 GUI_CRYSTAL_COUNT_POSITION = glm::vec3(-4.0f, -12.1f, 2.5f);
static const glm::vec3 GUI_CRYSTAL_COUNT_SCALE = glm::vec3(0.006f, 0.006f, 1.0f);

///------------------------------------------------------------------------------------------------

Scene::Scene()
    : mBox2dWorld(b2Vec2(0.0f, 0.0f))
    , mSceneUpdater(nullptr)
    , mTransitionParameters(nullptr)
    , mSceneRenderer(mBox2dWorld)
    , mPreFirstUpdate(true)
    , mSceneEditMode(false)
{
    FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_NAME);
    FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_MM_NAME);
    CreateCrossSceneInterfaceObjects();
}

///------------------------------------------------------------------------------------------------

Scene::~Scene()
{
    
}

///------------------------------------------------------------------------------------------------

std::string Scene::GetSceneStateDescription() const
{
    return "SOs: " + std::to_string(mSceneObjects.size()) + " bodies: " + std::to_string(mBox2dWorld.GetBodyCount()) + " enemies: " + (mSceneUpdater ? mSceneUpdater->VGetDescription() : "");
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<SceneObject>> Scene::GetSceneObject(const b2Body* body)
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](SceneObject& so)
    {
        return so.mBody == body;
    });
    
    if (findIter != mSceneObjects.end())
    {
        return std::optional<std::reference_wrapper<SceneObject>>{*findIter};
    }
    
    findIter = std::find_if(mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end(), [&](SceneObject& so)
    {
        return so.mBody == body;
    });
    
    if (findIter != mSceneObjectsToAdd.end())
    {
        return std::optional<std::reference_wrapper<SceneObject>>{*findIter};
    }
    
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<const SceneObject>> Scene::GetSceneObject(const b2Body* body) const
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const SceneObject& so)
    {
        return so.mBody == body;
    });
    
    if (findIter != mSceneObjects.end())
    {
        return std::optional<std::reference_wrapper<const SceneObject>>{*findIter};
    }
    
    findIter = std::find_if(mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end(), [&](const SceneObject& so)
    {
        return so.mBody == body;
    });
    
    if (findIter != mSceneObjectsToAdd.end())
    {
        return std::optional<std::reference_wrapper<const SceneObject>>{*findIter};
    }
    
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<SceneObject>> Scene::GetSceneObject(const strutils::StringId& sceneObjectName)
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](SceneObject& so)
    {
        return so.mName == sceneObjectName;
    });
    
    if (findIter != mSceneObjects.end())
    {
        return std::optional<std::reference_wrapper<SceneObject>>{*findIter};
    }
    
    findIter = std::find_if(mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end(), [&](SceneObject& so)
    {
        return so.mName == sceneObjectName;
    });
    
    if (findIter != mSceneObjectsToAdd.end())
    {
        return std::optional<std::reference_wrapper<SceneObject>>{*findIter};
    }
    
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<const SceneObject>> Scene::GetSceneObject(const strutils::StringId& sceneObjectName) const
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const SceneObject& so)
    {
        return so.mName == sceneObjectName;
    });
    
    if (findIter != mSceneObjects.end())
    {
        return std::optional<std::reference_wrapper<const SceneObject>>{*findIter};
    }
    
    findIter = std::find_if(mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end(), [&](const SceneObject& so)
    {
        return so.mName == sceneObjectName;
    });
    
    if (findIter != mSceneObjectsToAdd.end())
    {
        return std::optional<std::reference_wrapper<const SceneObject>>{*findIter};
    }
    
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void Scene::AddOverlayController(const float darkeningSpeed, const float maxDarkeningValue, const bool pauseAtMidPoint, FullScreenOverlayController::CallbackType midwayCallback /* nullptr */, FullScreenOverlayController::CallbackType completionCallback /* nullptr */)
{
    mOverlayController = std::make_unique<FullScreenOverlayController>(*this, darkeningSpeed, maxDarkeningValue, pauseAtMidPoint, midwayCallback, completionCallback);
}

///------------------------------------------------------------------------------------------------

void Scene::ResumeOverlayController()
{
    if (mOverlayController)
    {
        mOverlayController->Resume();
    }
}

///------------------------------------------------------------------------------------------------

const std::vector<SceneObject>& Scene::GetSceneObjects() const
{
    return mSceneObjects;
}

///------------------------------------------------------------------------------------------------

LightRepository& Scene::GetLightRepository()
{
    return mLightRepository;
}

///------------------------------------------------------------------------------------------------

const LightRepository& Scene::GetLightRepository() const
{
    return mLightRepository;
}

///------------------------------------------------------------------------------------------------

void Scene::AddSceneObject(SceneObject&& sceneObject)
{
    if (mPreFirstUpdate)
    {
        mSceneObjects.emplace_back(std::move(sceneObject));
    }
    else
    {
        mSceneObjectsToAdd.emplace_back(std::move(sceneObject));
    }
        
}

///------------------------------------------------------------------------------------------------

void Scene::RemoveAllSceneObjectsWithName(const strutils::StringId& name)
{
    assert(!mPreFirstUpdate);
    mNamesOfSceneObjectsToRemove.push_back(name);
}

///------------------------------------------------------------------------------------------------

void Scene::ChangeScene(const TransitionParameters& transitionParameters)
{
    // Need to hold a copy of the transition parameters as in the use case of an overlay
    // the scene creation needs to be deferred, and we don't want the stored lambda
    // to have an invalid reference to this temporary
    mTransitionParameters = std::make_unique<TransitionParameters>(transitionParameters.mSceneType, transitionParameters.mSceneNameToTransitionTo, transitionParameters.mUseOverlay);
    
    auto sceneCreationLambda = [&]()
    {
        std::vector<SceneObject> crossSceneSceneObjects;
        for (auto& so: mSceneObjects)
        {
            if (so.mCrossSceneLifetime)
            {
                crossSceneSceneObjects.push_back(std::move(so));
            }
            else
            {
                if (so.mBody)
                {
                    delete static_cast<strutils::StringId*>(so.mBody->GetUserData());
                    mBox2dWorld.DestroyBody(so.mBody);
                }
            }
        }
        mSceneObjects.clear();
        mSceneObjectsToAdd.clear();
        
        GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera());
        GameSingletons::SetCameraForSceneObjectType(SceneObjectType::GUIObject, Camera());
        
        switch (mTransitionParameters->mSceneType)
        {
            case SceneType::MAP:
            {
                mSceneUpdater = std::make_unique<MapUpdater>(*this);
            } break;
                
            case SceneType::LAB:
            {
                mSceneUpdater = std::make_unique<LabUpdater>(*this);
            } break;
                
            case SceneType::STATS_UPGRADE:
            {
                mSceneUpdater = std::make_unique<StatsUpgradeUpdater>(*this);
            } break;
                
            case SceneType::LEVEL:
            {
                LevelDataLoader levelDataLoader;
                auto levelDef = levelDataLoader.LoadLevel(mTransitionParameters->mSceneNameToTransitionTo);
                auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
                
                for (auto& enemyType: levelDef.mEnemyTypes)
                {
                    objectTypeDefRepo.LoadObjectTypeDefinition(enemyType);
                }
                
                for (const auto& camera: levelDef.mCameras)
                {
                    if (camera.mType == strutils::StringId("world_cam"))
                    {
                        GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera(camera.mLenseHeight));
                    }
                    else if (camera.mType == strutils::StringId("gui_cam"))
                    {
                        GameSingletons::SetCameraForSceneObjectType(SceneObjectType::GUIObject, Camera(camera.mLenseHeight));
                    }
                }
                
                mSceneUpdater = std::make_unique<LevelUpdater>(*this, mBox2dWorld, std::move(levelDef));
            } break;
        }
        
        
        for (auto& so: crossSceneSceneObjects)
        {
            AddSceneObject(std::move(so));
        }
    };
    
    if (mTransitionParameters->mUseOverlay)
    {
        AddOverlayController(game_constants::FULL_SCREEN_OVERLAY_TRANSITION_DARKENING_SPEED, game_constants::FULL_SCREEN_OVERLAY_TRANSITION_MAX_ALPHA, false, sceneCreationLambda);
    }
    else
    {
        sceneCreationLambda();
    }
}

///------------------------------------------------------------------------------------------------

void Scene::OnAppStateChange(Uint32 event)
{
    if (mSceneUpdater)
    {
        mSceneUpdater->VOnAppStateChange(event);
    }
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateScene(const float dtMillis)
{
    mPreFirstUpdate = false;
    
    if (mOverlayController)
    {
        mOverlayController->Update(dtMillis);
        if (mOverlayController->IsFinished())
        {
            RemoveAllSceneObjectsWithName(game_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME);
            mOverlayController = nullptr;
        }
    }
    
    if (mSceneEditMode && (!mSceneUpdater || mSceneUpdater->VGetStateMachineActiveStateName() != DebugConsoleGameState::STATE_NAME))
    {
        UpdateOnSceneEditModeOn(dtMillis);
    }
    else
    {
        if (mSceneUpdater)
        {
            if (mSceneUpdater->VUpdate(mSceneObjects, dtMillis * GameSingletons::GetGameSpeedMultiplier()) == PostStateUpdateDirective::CONTINUE)
            {
                UpdateCrossSceneInterfaceObjects(dtMillis);
            }
        }
    }
    
    for (const auto& name: mNamesOfSceneObjectsToRemove)
    {
        do
        {
            auto iter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const SceneObject& so)
            {
                return so.mName == name;
            });
            
            if (iter == mSceneObjects.end())
            {
                break;
            }
            
            if (iter->mBody)
            {
                delete static_cast<strutils::StringId*>(iter->mBody->GetUserData());
                mBox2dWorld.DestroyBody(iter->mBody);
            }
            
            auto sizeBefore = mSceneObjects.size();
            mSceneObjects.erase(iter);
            auto sizeNow = mSceneObjects.size();
            assert(sizeBefore = sizeNow + 1);
        } while (true);
    }
    
    mNamesOfSceneObjectsToRemove.clear();
    
    std::move(mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end(), std::back_inserter(mSceneObjects));
    mSceneObjectsToAdd.clear();
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateCrossSceneInterfaceObjects(const float dtMillis)
{
    // Player Health Bar update
    auto playerHealthBarFrameSoOpt = GetSceneObject(game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto playerHealthBarSoOpt = GetSceneObject(game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME);
    auto playerHealthBarTextSoOpt = GetSceneObject(game_constants::PLAYER_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME);

    if (playerHealthBarSoOpt && playerHealthBarFrameSoOpt && playerHealthBarTextSoOpt)
    {
        auto& healthBarFrameSo = playerHealthBarFrameSoOpt->get();
        auto& healthBarSo = playerHealthBarSoOpt->get();
        auto& healthBarTextSo = playerHealthBarTextSoOpt->get();
        
        healthBarSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mPosition.z = game_constants::PLAYER_HEALTH_BAR_Z;
        
        healthBarFrameSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        
        float healthPerc =  GameSingletons::GetPlayerCurrentHealth()/GameSingletons::GetPlayerMaxHealth();
        
        auto displayedHealthPercentage = GameSingletons::GetPlayerDisplayedHealth()/GameSingletons::GetPlayerMaxHealth();
        
        healthBarSo.mScale.x = game_constants::PLAYER_HEALTH_BAR_SCALE.x * displayedHealthPercentage;
        healthBarSo.mPosition.x -= (1.0f - displayedHealthPercentage)/game_constants::HEALTH_BAR_POSITION_DIVISOR_MAGIC * game_constants::PLAYER_HEALTH_BAR_SCALE.x;
        
        if (healthPerc < displayedHealthPercentage)
        {
            GameSingletons::SetPlayerDisplayedHealth((displayedHealthPercentage - game_constants::HEALTH_LOST_SPEED * dtMillis) * GameSingletons::GetPlayerMaxHealth());
            if (GameSingletons::GetPlayerDisplayedHealth()/GameSingletons::GetPlayerMaxHealth() <= healthPerc)
            {
                GameSingletons::SetPlayerDisplayedHealth(healthPerc * GameSingletons::GetPlayerMaxHealth());
            }
        }
        else if (healthPerc > displayedHealthPercentage)
        {
            GameSingletons::SetPlayerDisplayedHealth((displayedHealthPercentage + game_constants::HEALTH_LOST_SPEED * dtMillis) * GameSingletons::GetPlayerMaxHealth());
            if (GameSingletons::GetPlayerDisplayedHealth()/GameSingletons::GetPlayerMaxHealth() >= healthPerc)
            {
                GameSingletons::SetPlayerDisplayedHealth(healthPerc * GameSingletons::GetPlayerMaxHealth());
            }
        }
        
        healthBarTextSo.mText = std::to_string(static_cast<int>(GameSingletons::GetPlayerDisplayedHealth()));
        healthBarTextSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION + game_constants::HEALTH_BAR_TEXT_OFFSET;
        healthBarTextSo.mPosition.x -= (healthBarTextSo.mText.size() * 0.5f)/2;
    }
    
    // Crystal Count update
    auto crystalCountSoOpt = GetSceneObject(game_constants::GUI_CRYSTAL_COUNT_SCENE_OBJECT_NAME);
    if (crystalCountSoOpt)
    {
        auto& crystalCountSo = crystalCountSoOpt->get();
        if (GameSingletons::GetDisplayedCrystalCount() > GameSingletons::GetCrystalCount())
        {
            GameSingletons::SetDisplayedCrystalCount(GameSingletons::GetDisplayedCrystalCount() - game_constants::CRYSTAL_COUNT_CHANGE_SPEED * dtMillis);
            if (GameSingletons::GetDisplayedCrystalCount() <= GameSingletons::GetCrystalCount())
            {
                GameSingletons::SetDisplayedCrystalCount(GameSingletons::GetCrystalCount());
            }
        }
        else if (GameSingletons::GetDisplayedCrystalCount() < GameSingletons::GetCrystalCount())
        {
            GameSingletons::SetDisplayedCrystalCount(GameSingletons::GetDisplayedCrystalCount() + game_constants::CRYSTAL_COUNT_CHANGE_SPEED * dtMillis);
            if (GameSingletons::GetDisplayedCrystalCount() <= GameSingletons::GetCrystalCount())
            {
                GameSingletons::SetDisplayedCrystalCount(GameSingletons::GetCrystalCount());
            }
        }
        
        crystalCountSo.mText = std::to_string(static_cast<int>(GameSingletons::GetDisplayedCrystalCount()));
        crystalCountSo.mPosition = GUI_CRYSTAL_COUNT_POSITION;
        crystalCountSo.mPosition.x -= (crystalCountSo.mText.size() * 0.5f)/3.0;
    }
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateOnSceneEditModeOn(const float dtMillis)
{
    auto& inputContext = GameSingletons::GetInputContext();
    static float previousPinchDistance = 0.0f;
    static bool previousMultiGestureActive = false;
    static glm::vec3 worldInitTouchPos, guiInitTouchPos;
    static glm::vec3 initTouchOffset;
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        std::vector<strutils::StringId> touchedSceneObjectNames;
        
        auto worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
        auto& worldCamera = worldCamOpt->get();
        
        auto guiCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
        auto& guiCamera = guiCamOpt->get();
        
        worldInitTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        guiInitTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, worldCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
            
        for (int i = 0; i < mSceneObjects.size(); ++i)
        {
            auto& so = mSceneObjects.at(i);
            so.mDebugEditSelected = false;
            
            if (so.mBody == nullptr)
            {
                if ((so.mSceneObjectType == SceneObjectType::GUIObject && scene_object_utils::IsPointInsideSceneObject(so, guiInitTouchPos)) ||
                    (so.mSceneObjectType == SceneObjectType::WorldGameObject && scene_object_utils::IsPointInsideSceneObject(so, worldInitTouchPos)))
                {
                    if (so.mName.isEmpty())
                    {
                        so.mName = strutils::StringId(std::to_string(SDL_GetTicks64()) + std::to_string(i));
                    }
                    
                    if (so.mPosition.z > 0.0f && so.mName != SCENE_EDIT_RESULT_TEXT_NAME_1 && so.mName != SCENE_EDIT_RESULT_TEXT_NAME_2)
                    {
                        touchedSceneObjectNames.push_back(so.mName);
                    }
                }
            }
        }
        
        std::sort(touchedSceneObjectNames.begin(), touchedSceneObjectNames.end(), [&](const strutils::StringId& lhs, const strutils::StringId& rhs)
        {
            return GetSceneObject(lhs)->get().mPosition.z > GetSceneObject(rhs)->get().mPosition.z;
        });
        
        if (touchedSceneObjectNames.size() > 0)
        {
            auto& so = GetSceneObject(touchedSceneObjectNames.front())->get();
            so.mDebugEditSelected = true;
            auto initTouchPos = so.mSceneObjectType == SceneObjectType::GUIObject ? guiInitTouchPos : worldInitTouchPos;
            initTouchOffset = so.mBody ? initTouchPos - math::Box2dVec2ToGlmVec3(so.mBody->GetWorldCenter()) : initTouchPos - so.mPosition;
            
            SetSceneEditResultMessage(so.mBody ? math::Box2dVec2ToGlmVec3(so.mBody->GetWorldCenter()) : so.mPosition, so.mScale);
        }
    }
    else if (inputContext.mEventType == SDL_FINGERMOTION)
    {
        auto selectedSoIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [](const SceneObject& so)
        {
            return so.mDebugEditSelected;
        });
        
        if (selectedSoIter != mSceneObjects.end())
        {
            auto& so = *selectedSoIter;
            
            // Scale selected object
            if (inputContext.mPinchDistance > 0.0f && previousPinchDistance > 0.0f && inputContext.mMultiGestureActive)
            {
                so.mScale += dtMillis * (GameSingletons::GetInputContext().mPinchDistance - previousPinchDistance) * 0.3f;
                SetSceneEditResultMessage(so.mPosition, so.mScale);
            }
            // Translate selected object
            else if (inputContext.mMultiGestureActive == false && previousMultiGestureActive == false)
            {
                auto& camera = GameSingletons::GetCameraForSceneObjectType(so.mSceneObjectType)->get();
                auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, camera.GetViewMatrix(), camera.GetProjMatrix());
                
                if (so.mBody)
                {
                    so.mBody->SetTransform(math::GlmVec3ToBox2dVec2(touchPos - initTouchOffset), so.mBody->GetAngle());
                    SetSceneEditResultMessage(math::Box2dVec2ToGlmVec3(so.mBody->GetWorldCenter()), so.mScale);
                }
                else
                {
                    so.mPosition.x = touchPos.x - initTouchOffset.x;
                    so.mPosition.y = touchPos.y - initTouchOffset.y;
                    SetSceneEditResultMessage(so.mPosition, so.mScale);
                }
            }
            
            // Keep track of previous finger pinch distance
            previousPinchDistance = GameSingletons::GetInputContext().mPinchDistance;
        }
    }
    
    previousMultiGestureActive = inputContext.mMultiGestureActive;
}

///------------------------------------------------------------------------------------------------

void Scene::RenderScene()
{
    mSceneRenderer.Render(mSceneObjects, mLightRepository);
}

///------------------------------------------------------------------------------------------------

void Scene::SetSceneRendererPhysicsDebugMode(const bool debugMode)
{
    mSceneRenderer.SetPhysicsDebugMode(debugMode);
}

///------------------------------------------------------------------------------------------------

void Scene::SetSceneEditMode(const bool editMode)
{
    mSceneEditMode = editMode;
    
    RemoveAllSceneObjectsWithName(SCENE_EDIT_RESULT_TEXT_NAME_1);
    RemoveAllSceneObjectsWithName(SCENE_EDIT_RESULT_TEXT_NAME_2);
    for (auto& so: mSceneObjects)
    {
        so.mDebugEditSelected = false;
    }
}

///------------------------------------------------------------------------------------------------

void Scene::SetSceneEditResultMessage(const glm::vec3& position, const glm::vec3& scale)
{
    std::stringstream positionString;
    positionString << "Position: ";
    positionString << std::fixed << std::setprecision(4) << position.x << ", ";
    positionString << std::fixed << std::setprecision(4) << position.y << ", ";
    positionString << std::fixed << std::setprecision(4) << position.z;
    
    std::stringstream scaleString;
    scaleString << "Scale: ";
    scaleString << std::fixed << std::setprecision(4) << scale.x << ", ";
    scaleString << std::fixed << std::setprecision(4) << scale.y << ", ";
    scaleString << std::fixed << std::setprecision(4) << scale.z;
    
    auto sceneEditResultMessage1SoOpt = GetSceneObject(SCENE_EDIT_RESULT_TEXT_NAME_1);
    if (sceneEditResultMessage1SoOpt)
    {
        sceneEditResultMessage1SoOpt->get().mText = positionString.str();
    }
    else
    {
        SceneObject textSo;
        textSo.mPosition = glm::vec3(-5.5, 0.0f, 4.0f);
        textSo.mScale = glm::vec3(0.007f, 0.007f, 1.0f);
        
        auto& resService = resources::ResourceLoadingService::GetInstance();
        textSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(textSo.mScale), false);
        textSo.mName = SCENE_EDIT_RESULT_TEXT_NAME_1;
        textSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        textSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        textSo.mText = positionString.str();
        AddSceneObject(std::move(textSo));
    }
    
    auto sceneEditResultMessage2SoOpt = GetSceneObject(SCENE_EDIT_RESULT_TEXT_NAME_2);
    if (sceneEditResultMessage2SoOpt)
    {
        sceneEditResultMessage2SoOpt->get().mText = scaleString.str();
    }
    else
    {
        SceneObject textSo;
        textSo.mPosition = glm::vec3(-5.5, -1.0f, 4.0f);
        textSo.mScale = glm::vec3(0.007f, 0.007f, 1.0f);
        
        auto& resService = resources::ResourceLoadingService::GetInstance();
        textSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(textSo.mScale), false);
        textSo.mName = SCENE_EDIT_RESULT_TEXT_NAME_2;
        textSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        textSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        textSo.mText = positionString.str();
        AddSceneObject(std::move(textSo));
    }
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void Scene::OpenDebugConsole()
{
    if (mSceneUpdater)
    {
        mSceneUpdater->VOpenDebugConsole();
    }
}
#endif

///------------------------------------------------------------------------------------------------

void Scene::CreateCrossSceneInterfaceObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Player Health Bar
    {
        SceneObject healthBarSo;
        healthBarSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mScale = game_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarSo.mName = game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME;
        healthBarSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(healthBarSo));
    }
    
    // Player Health Bar Frame
    {
        SceneObject healthBarFrameSo;
        healthBarFrameSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarFrameSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarFrameSo.mScale = game_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarFrameSo.mName = game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME;
        healthBarFrameSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(healthBarFrameSo));
    }
    
    // Crystsal Holder
    {
        SceneObject crystalHolder;
        crystalHolder.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTAL_HOLDER_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        crystalHolder.mSceneObjectType = SceneObjectType::GUIObject;
        crystalHolder.mPosition = GUI_CRYSTAL_COUNT_HOLDER_POSITION;
        crystalHolder.mScale = GUI_CRYSTAL_COUNT_HOLDER_SCALE;
        crystalHolder.mCrossSceneLifetime = true;
        AddSceneObject(std::move(crystalHolder));
    }
    
    // Crystsal GUI icon
    {
        SceneObject crystalIconSo;
        crystalIconSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false);
        crystalIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalIconSo.mPosition = game_constants::GUI_CRYSTAL_POSITION;
        crystalIconSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
        crystalIconSo.mName = game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME;
        crystalIconSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(crystalIconSo));
    }
    
    // Crystal Count Text
    {
        SceneObject crystalCountSo;
        crystalCountSo.mPosition = GUI_CRYSTAL_COUNT_POSITION;
        crystalCountSo.mScale = GUI_CRYSTAL_COUNT_SCALE;
        crystalCountSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), GUI_CRYSTAL_COUNT_SCALE, false);
        crystalCountSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        crystalCountSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalCountSo.mName = game_constants::GUI_CRYSTAL_COUNT_SCENE_OBJECT_NAME;
        crystalCountSo.mText = std::to_string(GameSingletons::GetCrystalCount());
        crystalCountSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(crystalCountSo));
    }
    
    auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
    auto& playerDef = typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
    
    GameSingletons::SetPlayerDisplayedHealth(playerDef.mHealth);
    GameSingletons::SetPlayerMaxHealth(playerDef.mHealth);
    GameSingletons::SetPlayerCurrentHealth(playerDef.mHealth);
    GameSingletons::SetPlayerAttackStat(playerDef.mDamage);
    GameSingletons::SetPlayerMovementSpeedStat(1.0f);
    GameSingletons::SetPlayerBulletSpeedStat(1.0f);
    GameSingletons::SetCrystalCount(100);
    GameSingletons::SetDisplayedCrystalCount(100);
    
    // Player Health Bar Text
    {
        SceneObject healthBarTextSo;
        healthBarTextSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION + game_constants::HEALTH_BAR_TEXT_OFFSET;
        healthBarTextSo.mScale = game_constants::HEALTH_BAR_TEXT_SCALE;
        healthBarTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), game_constants::HEALTH_BAR_TEXT_SCALE, false);
        healthBarTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        healthBarTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarTextSo.mName = game_constants::PLAYER_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME;
        healthBarTextSo.mText = std::to_string(static_cast<int>(GameSingletons::GetPlayerDisplayedHealth()));
        healthBarTextSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(healthBarTextSo));
    }
}

///------------------------------------------------------------------------------------------------

