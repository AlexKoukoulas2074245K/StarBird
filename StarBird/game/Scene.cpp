///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "ChestRewardUpdater.h"
#include "EventUpdater.h"
#include "GameConstants.h"
#include "LabUpdater.h"
#include "MainMenuUpdater.h"
#include "MapUpdater.h"
#include "LevelUpdater.h"
#include "PersistenceUtils.h"
#include "PhysicsConstants.h"
#include "ResearchUpdater.h"
#include "SceneObjectUtils.h"
#include "StatsUpgradeUpdater.h"
#include "Sounds.h"
#include "ObjectTypeDefinitionRepository.h"
#include "states/DebugConsoleGameState.h"
#include "datarepos/WaveBlocksRepository.h"
#include "dataloaders/LevelDataLoader.h"
#include "dataloaders/UpgradesLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"
#include "../utils/ObjectiveCUtils.h"

#include <algorithm>
#include <Box2D/Box2D.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SCENE_EDIT_RESULT_TEXT_NAME_1 = strutils::StringId("SCENE_EDIT_RESULT_TEXT_1");
static const strutils::StringId SCENE_EDIT_RESULT_TEXT_NAME_2 = strutils::StringId("SCENE_EDIT_RESULT_TEXT_2");

static const glm::vec3 SCENE_EDIT_MAX_SO_SCALE_ELIGIBILITY = glm::vec3(15.0f, 15.0f, 15.0f);

static const glm::vec3 GUI_CRYSTAL_COUNT_HOLDER_SCALE = glm::vec3(2.5f, 3.5f, 1.0f);
static const glm::vec3 GUI_CRYSTAL_COUNT_HOLDER_POSITION = glm::vec3(-4.2f, -10.9f, 2.0f);

static const glm::vec3 GUI_CRYSTAL_COUNT_POSITION = glm::vec3(-4.0f, -12.1f, 2.5f);
static const glm::vec3 GUI_CRYSTAL_COUNT_SCALE = glm::vec3(0.006f, 0.006f, 1.0f);

static const glm::vec3 GUI_SETTINGS_ICON_POSITION = glm::vec3(5.0f, -12.0f, 2.5f);
static const glm::vec3 GUI_SETTINGS_ICON_SCALE = glm::vec3(1.31f, 1.31f, 1.0f);

static const float GUI_MIN_ALPHA = 0.1f;
static const float GUI_FADEOUT_LEFT_THRESHOLD = -1.5f;
static const float GUI_FADEOUT_LEFT_Y_THRESHOLD = -7.8f;
static const float GUI_FADEOUT_RIGHT_Y_THRESHOLD = -9.8f;

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> GUI_ELEMENT_NAMES =
{
    game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME,
    game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME,
    game_constants::PLAYER_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME,
    game_constants::GUI_CRYSTAL_HOLDER_SCENE_OBJECT_NAME,
    game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME,
    game_constants::GUI_CRYSTAL_COUNT_SCENE_OBJECT_NAME,
    game_constants::GUI_SETTINGS_ICON_SCENE_OBJECT_NAME
    
};

///------------------------------------------------------------------------------------------------

Scene::Scene()
    : mBox2dWorld(b2Vec2(0.0f, 0.0f))
    , mSceneUpdater(nullptr)
    , mTransitionParameters(nullptr)
    , mSceneRenderer(mBox2dWorld)
    , mPreFirstUpdate(true)
    , mSceneEditMode(false)
    , mProgressResetFlag(false)
{
    FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_NAME);
    FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_MM_NAME);
    
    GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera());
    GameSingletons::SetCameraForSceneObjectType(SceneObjectType::GUIObject, Camera());
    
    // Set fallback assets
    resources::ResourceLoadingService::GetInstance().SetFallbackTexture(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "debug.bmp");
    resources::ResourceLoadingService::GetInstance().SetFallbackMesh(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    resources::ResourceLoadingService::GetInstance().SetFallbackShader(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    
    CreateCrossSceneInterfaceObjects();
}

///------------------------------------------------------------------------------------------------

Scene::~Scene()
{
    HandleProgressReset();
}

///------------------------------------------------------------------------------------------------

std::string Scene::GetSceneStateDescription() const
{
    return "SOs: " + std::to_string(mSceneObjects.size()) + " bodies: " + std::to_string(mBox2dWorld.GetBodyCount()) + " scene description: " + (mSceneUpdater ? mSceneUpdater->VGetDescription() : "");
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
    if (sceneObject.mAnimation)
    {
        mAccumulatedResourcesForScene.insert(sceneObject.mAnimation->VGetCurrentTextureResourceId());
        mAccumulatedResourcesForScene.insert(sceneObject.mAnimation->VGetCurrentMeshResourceId());
        mAccumulatedResourcesForScene.insert(sceneObject.mAnimation->VGetCurrentShaderResourceId());
    }
    
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

void Scene::SetProgressResetFlag()
{
    mProgressResetFlag = true;
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
        HandleProgressReset();
        
        std::vector<SceneObject> crossSceneSceneObjects;
        std::unordered_set<resources::ResourceId> lockedResourceIds;
        
        for (auto& so: mSceneObjects)
        {
            if (so.mCrossSceneLifetime)
            {
                lockedResourceIds.insert(so.mAnimation->VGetCurrentTextureResourceId());
                lockedResourceIds.insert(so.mAnimation->VGetCurrentShaderResourceId());
                lockedResourceIds.insert(so.mAnimation->VGetCurrentMeshResourceId());
                
                for (auto& extraAnimation: so.mExtraCompoundingAnimations)
                {
                    lockedResourceIds.insert(extraAnimation->VGetCurrentTextureResourceId());
                    lockedResourceIds.insert(extraAnimation->VGetCurrentShaderResourceId());
                    lockedResourceIds.insert(extraAnimation->VGetCurrentMeshResourceId());
                }
                
                crossSceneSceneObjects.push_back(std::move(so));
            }
            else if (so.mBody)
            {
                delete static_cast<strutils::StringId*>(so.mBody->GetUserData());
                mBox2dWorld.DestroyBody(so.mBody);
            }
        }
        
        for (const auto resourceId: mAccumulatedResourcesForScene)
        {
            if (lockedResourceIds.count(resourceId) == 0)
            {
                resources::ResourceLoadingService::GetInstance().UnloadResource(resourceId);
            }
        }
        
        mSceneObjects.clear();
        mSceneObjectsToAdd.clear();
        mAccumulatedResourcesForScene.clear();
        
        mLightRepository.RemoveAllLights();
        
        FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_NAME);
        FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_MM_NAME);
        
        GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera());
        GameSingletons::SetCameraForSceneObjectType(SceneObjectType::GUIObject, Camera());
        
        resources::ResourceLoadingService::GetInstance().SetFallbackTexture(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "debug.bmp");
        resources::ResourceLoadingService::GetInstance().SetFallbackMesh(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
        resources::ResourceLoadingService::GetInstance().SetFallbackShader(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
        
        switch (mTransitionParameters->mSceneType)
        {
            case SceneType::MAIN_MENU:
            {
                mSceneUpdater = std::make_unique<MainMenuUpdater>(*this);
            } break;
                 
            case SceneType::MAP:
            {
                mSceneUpdater = std::make_unique<MapUpdater>(*this);
            } break;
                
            case SceneType::LAB:
            {
                mSceneUpdater = std::make_unique<LabUpdater>(*this, mBox2dWorld);
            } break;
                
            case SceneType::EVENT:
            {
                mSceneUpdater = std::make_unique<EventUpdater>(*this, mBox2dWorld);
            } break;
                
            case SceneType::RESEARCH:
            {
                mSceneUpdater = std::make_unique<ResearchUpdater>(*this);
            } break;
                
            case SceneType::STATS_UPGRADE:
            {
                mSceneUpdater = std::make_unique<StatsUpgradeUpdater>(*this);
            } break;
                
            case SceneType::CHEST_REWARD:
            {
                mSceneUpdater = std::make_unique<ChestRewardUpdater>(*this, mBox2dWorld);
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
        
        SetHUDVisibility(mTransitionParameters->mSceneType != SceneType::MAIN_MENU);
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
    HandleProgressReset();
    
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
        
        if (healthPerc > 0.0f)
        {
            auto displayedHealthPercentage = GameSingletons::GetPlayerDisplayedHealth()/GameSingletons::GetPlayerMaxHealth();
            
            healthBarSo.mScale.x = game_constants::PLAYER_HEALTH_BAR_SCALE.x * displayedHealthPercentage;
            healthBarSo.mPosition.x -= (1.0f - displayedHealthPercentage)/game_constants::BAR_POSITION_DIVISOR_MAGIC * game_constants::PLAYER_HEALTH_BAR_SCALE.x;
            
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
                GameSingletons::SetPlayerDisplayedHealth((displayedHealthPercentage + game_constants::HEALTH_LOST_SPEED/3 * dtMillis) * GameSingletons::GetPlayerMaxHealth());
                if (GameSingletons::GetPlayerDisplayedHealth()/GameSingletons::GetPlayerMaxHealth() >= healthPerc)
                {
                    GameSingletons::SetPlayerDisplayedHealth(healthPerc * GameSingletons::GetPlayerMaxHealth());
                }
            }
        }
        else
        {
            healthBarFrameSo.mInvisible = true;
            healthBarSo.mInvisible = true;
            healthBarTextSo.mInvisible = true;
            return;
        }
        
        healthBarTextSo.mText = std::to_string(static_cast<int>(GameSingletons::GetPlayerDisplayedHealth()));
        
        if (GameSingletons::GetPlayerShieldHealth() > 0.0f)
        {
            healthBarTextSo.mText += "<" + std::to_string(static_cast<int>(GameSingletons::GetPlayerShieldHealth())) + ">";
        }
        
        glm::vec2 botLeftRect, topRightRect;
        scene_object_utils::GetSceneObjectBoundingRect(healthBarTextSo, botLeftRect, topRightRect);
        healthBarTextSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION + game_constants::BAR_TEXT_OFFSET;
        
        healthBarTextSo.mPosition.x -= (math::Abs(botLeftRect.x - topRightRect.x)/2.0f);
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
    
    // Settings button press check
    auto settingsButtonSoOpt = GetSceneObject(game_constants::GUI_SETTINGS_ICON_SCENE_OBJECT_NAME);
    const auto guiCameraOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    if (settingsButtonSoOpt && guiCameraOpt && GameSingletons::GetInputContext().mEventType == SDL_FINGERDOWN)
    {
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCameraOpt->get().GetViewMatrix(), guiCameraOpt->get().GetProjMatrix());
        if (scene_object_utils::IsPointInsideSceneObject(*settingsButtonSoOpt, touchPos))
        {
            if (mSceneUpdater)
            {
                mSceneUpdater->VOpenSettingsMenu();
            }
            
            objectiveC_utils::PlaySound(resources::ResourceLoadingService::RES_SOUNDS_ROOT + sounds::WHOOSH_SFX_PATH, false);
        }
    }
    
    // Fade In/Out cross scene GUI SOs depending on player state
    b2Vec2 playerPosition(0.0f, 0.0f);
    bool playerEntityExists = GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME).has_value();
    if (playerEntityExists)
    {
        playerPosition = GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME)->get().mBody->GetWorldCenter();
    }
    
    std::for_each(GUI_ELEMENT_NAMES.begin(), GUI_ELEMENT_NAMES.end(), [&](const strutils::StringId& name)
    {
        auto soOpt = GetSceneObject(name);
        if (soOpt)
        {
            auto& guiSceneObject = soOpt->get();
            
            if (playerEntityExists)
            {
                // Fade out threshold
                if ((playerPosition.x < GUI_FADEOUT_LEFT_THRESHOLD && playerPosition.y < GUI_FADEOUT_LEFT_Y_THRESHOLD) || (playerPosition.x >= GUI_FADEOUT_LEFT_THRESHOLD && playerPosition.y < GUI_FADEOUT_RIGHT_Y_THRESHOLD))
                {
                    guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= game_constants::TEXT_FADE_IN_ALPHA_SPEED * dtMillis;
                    if (guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= GUI_MIN_ALPHA)
                    {
                        guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = GUI_MIN_ALPHA;
                    }
                }
                // Fade in threshold
                else
                {
                    guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += game_constants::TEXT_FADE_IN_ALPHA_SPEED * dtMillis;
                    if (guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                    {
                        guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                    }
                }
            }
            else
            {
                guiSceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            }
        }
    });
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
        
        guiInitTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
            
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
                    
                    if (so.mScale.x < SCENE_EDIT_MAX_SO_SCALE_ELIGIBILITY.x && so.mScale.y < SCENE_EDIT_MAX_SO_SCALE_ELIGIBILITY.y && so.mScale.z < SCENE_EDIT_MAX_SO_SCALE_ELIGIBILITY.z && so.mName != SCENE_EDIT_RESULT_TEXT_NAME_1 && so.mName != SCENE_EDIT_RESULT_TEXT_NAME_2)
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
                if (so.mText.empty())
                {
                    so.mScale += dtMillis * (GameSingletons::GetInputContext().mPinchDistance - previousPinchDistance) * 0.3f;
                }
                else
                {
                    so.mScale += dtMillis * (GameSingletons::GetInputContext().mPinchDistance - previousPinchDistance) * 0.03f;
                }
                
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
        textSo.mSceneObjectType = SceneObjectType::GUIObject;
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
        textSo.mSceneObjectType = SceneObjectType::GUIObject;
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
        healthBarSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mScale = game_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarSo.mName = game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME;
        healthBarSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        healthBarSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(healthBarSo));
    }
    
    // Player Health Bar Frame
    {
        SceneObject healthBarFrameSo;
        healthBarFrameSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarFrameSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarFrameSo.mScale = game_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarFrameSo.mName = game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME;
        healthBarFrameSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        healthBarFrameSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(healthBarFrameSo));
    }
    
    // Player Health Bar Text
    {
        SceneObject healthBarTextSo;
        healthBarTextSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION + game_constants::BAR_TEXT_OFFSET;
        healthBarTextSo.mScale = game_constants::BAR_TEXT_SCALE;
        healthBarTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), game_constants::BAR_TEXT_SCALE, false);
        healthBarTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        healthBarTextSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarTextSo.mName = game_constants::PLAYER_HEALTH_BAR_TEXT_SCENE_OBJECT_NAME;
        healthBarTextSo.mText = std::to_string(static_cast<int>(GameSingletons::GetPlayerDisplayedHealth()));
        healthBarTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        healthBarTextSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(healthBarTextSo));
    }
    
    // Crystsal Holder
    {
        SceneObject crystalHolder;
        crystalHolder.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTAL_HOLDER_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        crystalHolder.mSceneObjectType = SceneObjectType::GUIObject;
        crystalHolder.mPosition = GUI_CRYSTAL_COUNT_HOLDER_POSITION;
        crystalHolder.mScale = GUI_CRYSTAL_COUNT_HOLDER_SCALE;
        crystalHolder.mName = game_constants::GUI_CRYSTAL_HOLDER_SCENE_OBJECT_NAME;
        crystalHolder.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        crystalHolder.mCrossSceneLifetime = true;
        AddSceneObject(std::move(crystalHolder));
    }
    
    // Crystal GUI icon
    {
        SceneObject crystalIconSo;
        crystalIconSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false);
        crystalIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalIconSo.mPosition = game_constants::GUI_CRYSTAL_POSITION;
        crystalIconSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
        crystalIconSo.mName = game_constants::GUI_CRYSTAL_ICON_SCENE_OBJECT_NAME;
        crystalIconSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        crystalIconSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(crystalIconSo));
    }
    
    // Crystal Count Text
    {
        SceneObject crystalCountSo;
        crystalCountSo.mPosition = GUI_CRYSTAL_COUNT_POSITION;
        crystalCountSo.mScale = GUI_CRYSTAL_COUNT_SCALE;
        crystalCountSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), GUI_CRYSTAL_COUNT_SCALE, false);
        crystalCountSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
        crystalCountSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalCountSo.mName = game_constants::GUI_CRYSTAL_COUNT_SCENE_OBJECT_NAME;
        crystalCountSo.mText = std::to_string(GameSingletons::GetCrystalCount());
        crystalCountSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        crystalCountSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(crystalCountSo));
    }
    
    // Settings Icon Text
    {
        SceneObject settingsIconSo;
        settingsIconSo.mPosition = GUI_SETTINGS_ICON_POSITION;
        settingsIconSo.mScale = GUI_SETTINGS_ICON_SCALE;
        settingsIconSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::SETTINGS_ICON_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), GUI_SETTINGS_ICON_SCALE, false);
        settingsIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        settingsIconSo.mName = game_constants::GUI_SETTINGS_ICON_SCENE_OBJECT_NAME;
        settingsIconSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        settingsIconSo.mCrossSceneLifetime = true;
        AddSceneObject(std::move(settingsIconSo));
    }
}

///------------------------------------------------------------------------------------------------

void Scene::SetHUDVisibility(const bool visibility)
{
    for (auto& sceneObject: mSceneObjects)
    {
        if (sceneObject.mCrossSceneLifetime && sceneObject.mName != game_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME)
        {
            sceneObject.mInvisible = !visibility;
        }
    }
    
    for (auto& sceneObject: mSceneObjectsToAdd)
    {
        if (sceneObject.mCrossSceneLifetime && sceneObject.mName != game_constants::FULL_SCREEN_OVERLAY_SCENE_OBJECT_NAME)
        {
            sceneObject.mInvisible = !visibility;
        }
    }
}

///------------------------------------------------------------------------------------------------

void Scene::HandleProgressReset()
{
    if (mProgressResetFlag)
    {
        persistence_utils::GenerateNewProgressSaveFile();
        mProgressResetFlag = false;
    }
}

///------------------------------------------------------------------------------------------------
