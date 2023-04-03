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
#include "dataloaders/LevelDataLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"

#include <algorithm>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

static const glm::vec3 GUI_CRYSTAL_COUNT_HOLDER_SCALE = glm::vec3(2.5f, 3.5f, 1.0f);
static const glm::vec3 GUI_CRYSTAL_COUNT_HOLDER_POSITION = glm::vec3(-4.2f, -10.9f, 0.0f);

static const glm::vec3 GUI_CRYSTAL_COUNT_POSITION = glm::vec3(-4.0f, -12.1f, 2.0f);
static const glm::vec3 GUI_CRYSTAL_COUNT_SCALE = glm::vec3(0.006f, 0.006f, 1.0f);

static const glm::vec3 GUI_CRYSTAL_POSITION = glm::vec3(-4.2f, -10.2f, 0.5f);
static const glm::vec3 GUI_CRYSTAL_SCALE = glm::vec3(0.6f, 0.6f, 0.6f);

static const float GUI_CRYSTAL_ROTATION_SPEED = 0.0004f;

///------------------------------------------------------------------------------------------------

Scene::Scene()
    : mBox2dWorld(b2Vec2(0.0f, 0.0f))
    , mSceneUpdater(nullptr)
    , mTransitionParameters(nullptr)
    , mSceneRenderer(mBox2dWorld)
    , mPreFirstUpdate(true)
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
    
    if (mSceneUpdater)
    {
        if (mSceneUpdater->VUpdate(mSceneObjects, dtMillis * GameSingletons::GetGameSpeedMultiplier()) == PostStateUpdateDirective::CONTINUE)
        {
            UpdateCrossSceneInterfaceObjects(dtMillis);
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
    auto playerSO = GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
    auto playerDef = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
    
    if (playerSO && playerDef)
    {
        GameSingletons::SetPlayerCurrentHealth(playerSO->get().mHealth);
        GameSingletons::SetPlayerMaxHealth(playerDef->get().mHealth);
    }
    
    
    
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
        crystalIconSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, GUI_CRYSTAL_ROTATION_SPEED, false);
        crystalIconSo.mSceneObjectType = SceneObjectType::GUIObject;
        crystalIconSo.mPosition = GUI_CRYSTAL_POSITION;
        crystalIconSo.mScale = GUI_CRYSTAL_SCALE;
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
    GameSingletons::SetPlayerDisplayedHealth(typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get().mHealth);
    GameSingletons::SetPlayerMaxHealth(typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get().mHealth);
    GameSingletons::SetPlayerCurrentHealth(typeDefRepo.GetObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get().mHealth);
    
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

