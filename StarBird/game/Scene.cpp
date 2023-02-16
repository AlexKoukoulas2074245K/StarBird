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
#include "ObjectTypeDefinitionRepository.h"
#include "dataloaders/LevelDataLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"

#include <algorithm>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

Scene::Scene()
    : mBox2dWorld(b2Vec2(0.0f, 0.0f))
    , mLevelUpdater(*this, mBox2dWorld)
    , mPreFirstUpdate(true)
{
}

///------------------------------------------------------------------------------------------------

std::string Scene::GetSceneStateDescription() const
{
    return "bodies: " + std::to_string(mBox2dWorld.GetBodyCount()) + " enemies: " + std::to_string(mLevelUpdater.GetWaveEnemyCount());
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<SceneObject>> Scene::GetSceneObject(const strutils::StringId& sceneObjectNameTag)
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](SceneObject& so)
    {
        return so.mNameTag == sceneObjectNameTag;
    });
    
    if (findIter != mSceneObjects.end())
    {
        return std::optional<std::reference_wrapper<SceneObject>>{*findIter};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<const SceneObject>> Scene::GetSceneObject(const strutils::StringId& sceneObjectNameTag) const
{
    auto findIter = std::find_if(mSceneObjects.cbegin(), mSceneObjects.cend(), [&](const SceneObject& so)
    {
        return so.mNameTag == sceneObjectNameTag;
    });
    
    if (findIter != mSceneObjects.cend())
    {
        return std::optional<std::reference_wrapper<const SceneObject>>{*findIter};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void Scene::AddSceneObject(SceneObject&& sceneObject)
{
    if (mPreFirstUpdate)
    {
        mSceneObjects.emplace_back(sceneObject);
    }
    else
    {
        mSceneObjectsToAdd.emplace_back(sceneObject);
    }
        
}

///------------------------------------------------------------------------------------------------

void Scene::RemoveAllSceneObjectsWithNameTag(const strutils::StringId& nameTag)
{
    assert(!mPreFirstUpdate);
    mNameTagsOfSceneObjectsToRemove.push_back(nameTag);
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
    
    mBox2dWorld.Step(physics_constants::WORLD_STEP, physics_constants::WORLD_VELOCITY_ITERATIONS, physics_constants::WORLD_POSITION_ITERATIONS);
    
    mLevelUpdater.Update(mSceneObjects, dtMillis);
    
    for (const auto& nameTag: mNameTagsOfSceneObjectsToRemove)
    {
        auto iter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const SceneObject& so)
        {
            return so.mNameTag == nameTag;
        });
            
        if (iter != mSceneObjects.end())
        {
            if (iter->mBody)
            {
                mBox2dWorld.DestroyBody(iter->mBody);
            }
            auto sizeBefore = mSceneObjects.size();
            mSceneObjects.erase(iter);
            auto sizeNow = mSceneObjects.size();
            assert(sizeBefore = sizeNow + 1);
        }
    }
    mNameTagsOfSceneObjectsToRemove.clear();
    
    mSceneObjects.insert(mSceneObjects.end(), mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end());
    mSceneObjectsToAdd.clear();
}

///------------------------------------------------------------------------------------------------

void Scene::RenderScene()
{
    mSceneRenderer.Render(mSceneObjects);
}

///------------------------------------------------------------------------------------------------

void Scene::LoadLevelInvariantObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mCustomScale = game_object_constants::BACKGROUND_SCALE;
        bgSO.mCustomPosition.z = game_object_constants::BACKGROUND_Z;
        bgSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        bgSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::TEXTURE_OFFSET_SHADER_FILE_NAME);
        bgSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::BACKGROUND_TEXTURE_FILE_NAME);
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mNameTag = scene_object_constants::BACKGROUND_SCENE_OBJECT_NAME;
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
        
        SceneObject playerSO;
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(game_object_constants::PLAYER_INITIAL_POS.x, game_object_constants::PLAYER_INITIAL_POS.y);
        b2Body* body = mBox2dWorld.CreateBody(&bodyDef);
        
        b2PolygonShape dynamicBox;
        
        // Load projectiles
        resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::BULLET_TEXTURE_FILE_NAME);
        
        auto playerTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::PLAYER_TEXTURE_FILE_NAME);
        auto& playerTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(playerTextureResourceId);
        
        float textureAspect = playerTexture.GetDimensions().x/playerTexture.GetDimensions().y;
        dynamicBox.SetAsBox(1.0f, 1.0f/textureAspect);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = playerObjectDef.mDensity;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::PLAYER_CATEGORY_BIT;
        fixtureDef.filter.maskBits &= ~(physics_constants::PLAYER_BULLET_CATEGORY_BIT);
        body->CreateFixture(&fixtureDef);
        
        playerSO.mBody = body;
        playerSO.mHealth = playerObjectDef.mHealth;
        playerSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        playerSO.mTextureResourceId = playerTextureResourceId;
        playerSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        playerSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        playerSO.mCustomPosition.z = 0.0f;
        playerSO.mNameTag = scene_object_constants::PLAYER_SCENE_OBJECT_NAME;
        playerSO.mObjectFamilyTypeName = game_object_constants::PLAYER_OBJECT_TYPE_DEF_NAME;
        playerSO.mUseBodyForRendering = true;
        
        AddSceneObject(std::move(playerSO));
    }
    
    const auto& worldCamOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    assert(worldCamOpt);
    const auto& worldCam = worldCamOpt->get();
    
    // L_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(-worldCam.GetCameraLenseWidth()/2.0f, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, worldCam.GetCameraLenseHeight() * 4);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::GLOBAL_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mUseBodyForRendering = true;
        so.mInvisible = true;
        AddSceneObject(std::move(so));
    }
    
    // R_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(worldCam.GetCameraLenseWidth()/2, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, worldCam.GetCameraLenseHeight() * 4);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::GLOBAL_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mUseBodyForRendering = true;
        so.mInvisible = true;
        AddSceneObject(std::move(so));
    }

    // PLAYER_ONLY_BOT_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(0.0f, -worldCam.GetCameraLenseHeight()/2 + 1.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(worldCam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mUseBodyForRendering = true;
        so.mInvisible = true;
        AddSceneObject(std::move(so));
    }
    
    // ENEMY_ONLY_BOT_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(0.0f, -worldCam.GetCameraLenseHeight()/2 - 7.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(worldCam.GetCameraLenseWidth() * 4, 2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mUseBodyForRendering = true;
        so.mInvisible = true;
        AddSceneObject(std::move(so));
    }
    
    // BULLET_TOP_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(0.0f, worldCam.GetCameraLenseHeight()/2 + 1.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(worldCam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mUseBodyForRendering = true;
        so.mInvisible = true;
        AddSceneObject(std::move(so));
    }
    
    // Joystick
    {
        SceneObject joystickSO;
        joystickSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        joystickSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::JOYSTICK_TEXTURE_FILE_NAME);
        joystickSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        joystickSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickSO.mCustomScale = game_object_constants::JOYSTICK_SCALE;
        joystickSO.mNameTag = scene_object_constants::JOYSTICK_SCENE_OBJECT_NAME;
        joystickSO.mInvisible = true;
        AddSceneObject(std::move(joystickSO));
    }
    
    // Joystick Bounds
    {
        SceneObject joystickBoundsSO;
        joystickBoundsSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        joystickBoundsSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::JOYSTICK_BOUNDS_TEXTURE_FILE_NAME);
        joystickBoundsSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        joystickBoundsSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickBoundsSO.mCustomScale = game_object_constants::JOYSTICK_BOUNDS_SCALE;
        joystickBoundsSO.mNameTag = scene_object_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME;
        joystickBoundsSO.mInvisible = true;
        AddSceneObject(std::move(joystickBoundsSO));
    }
    
    // Player Health Bar
    {
        SceneObject healthBarSo;
        healthBarSo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        healthBarSo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::PLAYER_HEALTH_BAR_TEXTURE_FILE_NAME);
        healthBarSo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        healthBarSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarSo.mCustomPosition = game_object_constants::HEALTH_BAR_POSITION;
        healthBarSo.mCustomScale = game_object_constants::HEALTH_BAR_SCALE;
        healthBarSo.mNameTag = scene_object_constants::PLAYER_HEALTH_BAR_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(healthBarSo));
    }
    
    // Player Health Bar Frame
    {
        SceneObject healthBarFrameSo;
        healthBarFrameSo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + scene_object_constants::BASIC_SHADER_FILE_NAME);
        healthBarFrameSo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + scene_object_constants::PLAYER_HEALTH_BAR_FRAME_TEXTURE_FILE_NAME);
        healthBarFrameSo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + scene_object_constants::QUAD_MESH_FILE_NAME);
        healthBarFrameSo.mSceneObjectType = SceneObjectType::GUIObject;
        healthBarFrameSo.mCustomPosition = game_object_constants::HEALTH_BAR_POSITION;
        healthBarFrameSo.mCustomScale = game_object_constants::HEALTH_BAR_SCALE;
        healthBarFrameSo.mNameTag = scene_object_constants::PLAYER_HEALTH_BAR_FRAME_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(healthBarFrameSo));
    }
    
    FontRepository::GetInstance().LoadFont(strutils::StringId("font"));
}

///------------------------------------------------------------------------------------------------
