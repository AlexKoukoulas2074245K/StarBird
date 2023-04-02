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
#include "ObjectTypeDefinitionRepository.h"
#include "dataloaders/LevelDataLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"

#include <algorithm>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

Scene::Scene()
    : mBox2dWorld(b2Vec2(0.0f, 0.0f))
    , mSceneUpdater(nullptr)
    , mTransitionParameters(nullptr)
    , mSceneRenderer(mBox2dWorld)
    , mPreFirstUpdate(true)
{
    FontRepository::GetInstance().LoadFont(game_constants::DEFAULT_FONT_NAME);
    CreateCrossSceneInterfaceObjects();
}

///------------------------------------------------------------------------------------------------

Scene::~Scene()
{
    
}

///------------------------------------------------------------------------------------------------

std::string Scene::GetSceneStateDescription() const
{
    return "SOs: " + std::to_string(mSceneObjects.size()) + " bodies: " + std::to_string(mBox2dWorld.GetBodyCount()) + " enemies: " + (mSceneUpdater ? mSceneUpdater->GetDescription() : "");
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
        mSceneUpdater->OnAppStateChange(event);
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
    
    if (mSceneUpdater)
    {
        mSceneUpdater->Update(mSceneObjects, dtMillis * GameSingletons::GetGameSpeedMultiplier());
    }
    
    UpdateCrossSceneInterfaceObjects(dtMillis);
    
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
    auto playerSO = GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
    
    if (playerSO && playerDef)
    {
        GameSingletons::SetPlayerCurrentHealth(playerSO->get().mHealth);
        GameSingletons::SetPlayerMaxHealth(playerDef->get().mHealth);
    }
    
    auto playerHealthBarFrameSoOpt = GetSceneObject(game_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME);
    auto playerHealthBarSoOpt = GetSceneObject(game_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME);
    
    if (playerHealthBarSoOpt && playerHealthBarFrameSoOpt)
    {
        auto& healthBarFrameSo = playerHealthBarFrameSoOpt->get();
        auto& healthBarSo = playerHealthBarSoOpt->get();
        
        healthBarSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mPosition.z = game_constants::PLAYER_HEALTH_BAR_Z;
        
        healthBarFrameSo.mPosition = game_constants::PLAYER_HEALTH_BAR_POSITION;
        
        float healthPerc =  GameSingletons::GetPlayerCurrentHealth()/GameSingletons::GetPlayerMaxHealth();
        
        healthBarSo.mScale.x = game_constants::PLAYER_HEALTH_BAR_SCALE.x * GameSingletons::GetPlayerDisplayedHealth();
        healthBarSo.mPosition.x -= (1.0f - GameSingletons::GetPlayerDisplayedHealth())/game_constants::HEALTH_BAR_POSITION_DIVISOR_MAGIC * game_constants::PLAYER_HEALTH_BAR_SCALE.x;
        
        if (healthPerc < GameSingletons::GetPlayerDisplayedHealth())
        {
            GameSingletons::SetPlayerDisplayedHealth(GameSingletons::GetPlayerDisplayedHealth() - game_constants::HEALTH_LOST_SPEED * dtMillis);
            if (GameSingletons::GetPlayerDisplayedHealth() <= healthPerc)
            {
                GameSingletons::SetPlayerDisplayedHealth(healthPerc);
            }
        }
        else
        {
            GameSingletons::SetPlayerDisplayedHealth(GameSingletons::GetPlayerDisplayedHealth() + game_constants::HEALTH_LOST_SPEED * dtMillis);
            if (GameSingletons::GetPlayerDisplayedHealth() >= healthPerc)
            {
                GameSingletons::SetPlayerDisplayedHealth(healthPerc);
            }
        }
    }
    else
    {
        playerHealthBarFrameSoOpt->get().mInvisible = true;
        playerHealthBarSoOpt->get().mInvisible = true;
    }
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

#ifdef DEBUG
void Scene::OpenDebugConsole()
{
    if (mSceneUpdater)
    {
        mSceneUpdater->OpenDebugConsole();
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
    
    auto playerSO = GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    
    if (playerSO)
    {
        GameSingletons::SetPlayerDisplayedHealth(playerSO->get().mHealth);
    }
}

///------------------------------------------------------------------------------------------------

