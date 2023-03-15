///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameSingletons.h"
#include "Scene.h"
#include "GameObjectConstants.h"
#include "PhysicsConstants.h"
#include "SceneObjectConstants.h"
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
    , mLevelUpdater(*this, mBox2dWorld)
    , mSceneRenderer(mBox2dWorld)
    , mPreFirstUpdate(true)
{
}

///------------------------------------------------------------------------------------------------

std::string Scene::GetSceneStateDescription() const
{
    return "SOs: " + std::to_string(mSceneObjects.size()) + " bodies: " + std::to_string(mBox2dWorld.GetBodyCount()) + " enemies: " + std::to_string(mLevelUpdater.GetWaveEnemyCount());
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

void Scene::FreezeAllPhysicsBodies()
{
    for (auto& so: mSceneObjectsToAdd)
    {
        if (so.mBody)
        {
            so.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        }
    }
    
    for (auto& so: mSceneObjects)
    {
        if (so.mBody)
        {
            so.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        }
    }
}

///------------------------------------------------------------------------------------------------

void Scene::LoadLevel(const std::string& levelName)
{
    LevelDataLoader levelDataLoader;
    auto levelDef = levelDataLoader.LoadLevel(levelName);
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
    
    LoadLevelInvariantObjects();
    mLevelUpdater.InitLevel(std::move(levelDef));
}

///------------------------------------------------------------------------------------------------

void Scene::OnAppStateChange(Uint32 event)
{
    mLevelUpdater.OnAppStateChange(event);
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateScene(const float dtMillis)
{
    mPreFirstUpdate = false;
    
    mLevelUpdater.Update(mSceneObjects, dtMillis * GameSingletons::GetGameSpeedMultiplier());
    
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
    mLevelUpdater.OpenDebugConsole();
}
#endif

///------------------------------------------------------------------------------------------------

void Scene::CreateLevelWalls(const Camera& cam, const bool invisible)
{
    // L_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(-cam.GetCameraLenseWidth()/2.0f, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, cam.GetCameraLenseHeight() * 4);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::GLOBAL_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_object_constants::WALL_Z;
        so.mName = scene_object_constants::WALL_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(so));
    }
    
    // R_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(cam.GetCameraLenseWidth()/2, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, cam.GetCameraLenseHeight() * 4);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::GLOBAL_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_object_constants::WALL_Z;
        so.mName = scene_object_constants::WALL_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(so));
    }

    // PLAYER_ONLY_BOT_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, -cam.GetCameraLenseHeight()/2 + 1.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_object_constants::WALL_Z;
        so.mName = scene_object_constants::WALL_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(so));
    }
    
    // ENEMY_ONLY_BOT_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, -cam.GetCameraLenseHeight()/2 - 7.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth() * 4, 2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mInvisible = invisible;
        so.mPosition.z = game_object_constants::WALL_Z;
        so.mName = scene_object_constants::WALL_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(so));
    }
    
    // BULLET_TOP_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.type = b2_staticBody;
        wallBodyDef.position.Set(0.0f, cam.GetCameraLenseHeight()/2);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.filter.categoryBits = physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mAnimation = std::make_unique<SingleFrameAnimation>(resources::ResourceLoadingService::FALLBACK_TEXTURE_ID, resources::ResourceLoadingService::FALLBACK_MESH_ID, resources::ResourceLoadingService::FALLBACK_SHADER_ID, glm::vec3(1.0f), true);
        so.mInvisible = invisible;
        so.mPosition.z = game_object_constants::WALL_Z;
        so.mName = scene_object_constants::WALL_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

void Scene::LoadLevelInvariantObjects()
{
    mLightRepository.AddLight(LightType::AMBIENT_LIGHT, strutils::StringId("AMBIENT_LIGHT"), game_object_constants::AMBIENT_LIGHT_COLOR, glm::vec3(0.0f), 0.0f);
    
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_object_constants::BACKGROUND_SCALE;
        bgSO.mPosition.z = game_object_constants::BACKGROUND_Z;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::TEXTURE_OFFSET_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mName = scene_object_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[scene_object_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = true;
        AddSceneObject(std::move(bgSO));
    }
    
    // Player
    {
        auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
        typeDefRepo.LoadObjectTypeDefinition(game_object_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
        typeDefRepo.LoadObjectTypeDefinition(game_object_constants::PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_object_constants::BETTER_PLAYER_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_object_constants::MIRROR_IMAGE_BULLET_TYPE);
        typeDefRepo.LoadObjectTypeDefinition(game_object_constants::BETTER_MIRROR_IMAGE_BULLET_TYPE);
        
        auto& playerObjectDef = typeDefRepo.GetObjectTypeDefinition(game_object_constants::PLAYER_OBJECT_TYPE_DEF_NAME)->get();
        
        SceneObject playerSO = scene_object_utils::CreateSceneObjectWithBody(playerObjectDef, game_object_constants::PLAYER_INITIAL_POS, mBox2dWorld, scene_object_constants::PLAYER_SCENE_OBJECT_NAME);
        AddSceneObject(std::move(playerSO));
    }
    
    const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    assert(worldCamOpt);
    const auto& worldCam = worldCamOpt->get();
    
    CreateLevelWalls(worldCam, true);
    
    // Joystick
    {
        SceneObject joystickSO;
        joystickSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::JOYSTICK_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        joystickSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickSO.mScale = game_object_constants::JOYSTICK_SCALE;
        joystickSO.mName = scene_object_constants::JOYSTICK_SCENE_OBJECT_NAME;
        joystickSO.mInvisible = true;
        AddSceneObject(std::move(joystickSO));
    }
    
    // Joystick Bounds
    {
        SceneObject joystickBoundsSO;
        joystickBoundsSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::JOYSTICK_BOUNDS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        joystickBoundsSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickBoundsSO.mScale = game_object_constants::JOYSTICK_BOUNDS_SCALE;
        joystickBoundsSO.mName = scene_object_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME;
        joystickBoundsSO.mInvisible = true;
        AddSceneObject(std::move(joystickBoundsSO));
    }
    
    // Player Health Bar
    {
        SceneObject healthBarSo;
        healthBarSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarSo.mPosition = game_object_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarSo.mScale = game_object_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarSo.mName = scene_object_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(healthBarSo));
    }
    
    // Player Health Bar Frame
    {
        SceneObject healthBarFrameSo;
        healthBarFrameSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        healthBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarFrameSo.mPosition = game_object_constants::PLAYER_HEALTH_BAR_POSITION;
        healthBarFrameSo.mScale = game_object_constants::PLAYER_HEALTH_BAR_SCALE;
        healthBarFrameSo.mName = scene_object_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(healthBarFrameSo));
    }
    
    FontRepository::GetInstance().LoadFont(scene_object_constants::DEFAULT_FONT_NAME);
}

///------------------------------------------------------------------------------------------------

