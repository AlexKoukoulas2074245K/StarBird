///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "GameSingletons.h"
#include "Scene.h"
#include "GameObjectConstants.h"
#include "PhysicsConstants.h"
#include "SceneObjectConstants.h"
#include "ObjectTypeDefinitionRepository.h"
#include "dataloaders/LevelDataLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/TextureResource.h"

#include <algorithm>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

Scene::Scene()
    : mBox2dWorld(b2Vec2(0.0f, 0.0f))
    , mSceneUpdater(*this, mBox2dWorld)
    , mPreFirstUpdate(true)
{
}

///------------------------------------------------------------------------------------------------

int Scene::GetBodyCount() const
{
    return mBox2dWorld.GetBodyCount();
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
    
    for (const auto& enemy: levelDef.mWaves[0].mEnemies)
    {
        const auto& enemyDefOpt = objectTypeDefRepo.GetObjectTypeDefinition(enemy.mGameObjectEnemyType);
        if (!enemyDefOpt) continue;
        
        const auto& enemyDef = enemyDefOpt->get();
        
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(enemy.mPosition.x, enemy.mPosition.y);
        b2Body* body = mBox2dWorld.CreateBody(&bodyDef);
        body->SetLinearDamping(enemyDef.mLinearDamping);
        
        b2PolygonShape dynamicBox;
        auto& enemyTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(enemyDef.mTextureResourceId);
        
        float textureAspect = enemyTexture.GetDimensions().x/enemyTexture.GetDimensions().y;
        dynamicBox.SetAsBox(enemyDef.mSize, enemyDef.mSize/textureAspect);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = enemyDef.mDensity;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter = enemyDef.mContactFilter;
        body->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mObjectFamilyTypeName = enemy.mGameObjectEnemyType;
        so.mBody = body;
        so.mHealth = enemyDef.mHealth;
        
        so.mShaderResourceId = enemyDef.mShaderResourceId;
        so.mTextureResourceId = enemyDef.mTextureResourceId;
        so.mMeshResourceId = enemyDef.mMeshResourceId;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mCustomPosition.z = 0.0f;
        
        strutils::StringId nameTag;
        nameTag.fromAddress(so.mBody);
        
        so.mNameTag = nameTag;
        AddSceneObject(std::move(so));
    }
    
    LoadLevelInvariantObjects();
    
    mSceneUpdater.SetLevelProperties(std::move(levelDef));
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateScene(const float dtMilis)
{
    mPreFirstUpdate = false;
    for (const auto& nameTag: mNameTagsOfSceneObjectsToRemove)
    {
        auto iter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const SceneObject& so)
        {
            return so.mNameTag == nameTag;
        });
            
        if (iter != mSceneObjects.end())
        {
            mBox2dWorld.DestroyBody(iter->mBody);
            auto sizeBefore = mSceneObjects.size();
            mSceneObjects.erase(iter);
            auto sizeNow = mSceneObjects.size();
            assert(sizeBefore = sizeNow + 1);
        }
    }
    mNameTagsOfSceneObjectsToRemove.clear();
    
    mSceneObjects.insert(mSceneObjects.end(), mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end());
    mSceneObjectsToAdd.clear();
    
    mSceneUpdater.Update(mSceneObjects, dtMilis);
    
    mBox2dWorld.Step(physics_constants::WORLD_STEP, physics_constants::WORLD_VELOCITY_ITERATIONS, physics_constants::WORLD_POSITION_ITERATIONS);
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
        bgSO.mCustomScale = gameobject_constants::BACKGROUND_SCALE;
        bgSO.mCustomPosition.z = gameobject_constants::BACKGROUND_Z;
        bgSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        bgSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::TEXTURE_OFFSET_SHADER_FILE_NAME);
        bgSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::BACKGROUND_TEXTURE_FILE_NAME);
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mNameTag = sceneobject_constants::BACKGROUND_SCENE_OBJECT_NAME;
        AddSceneObject(std::move(bgSO));
    }
    
    // Player
    {
        SceneObject playerSO;
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(gameobject_constants::PLAYER_INITIAL_POS.x, gameobject_constants::PLAYER_INITIAL_POS.y);
        b2Body* body = mBox2dWorld.CreateBody(&bodyDef);
        
        b2PolygonShape dynamicBox;
        
        // Load projectiles
        resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::BULLET_TEXTURE_FILE_NAME);
        
        auto playerTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::PLAYER_TEXTURE_FILE_NAME);
        auto& playerTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(playerTextureResourceId);
        
        float textureAspect = playerTexture.GetDimensions().x/playerTexture.GetDimensions().y;
        dynamicBox.SetAsBox(1.0f, 1.0f/textureAspect);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::PLAYER_CATEGORY_BIT;
        fixtureDef.filter.maskBits &= (~(physics_constants::PLAYER_CATEGORY_BIT) | physics_constants::ENEMY_CATEGORY_BIT);
        body->CreateFixture(&fixtureDef);
        
        playerSO.mBody = body;
        playerSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        playerSO.mTextureResourceId = playerTextureResourceId;
        playerSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        playerSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        playerSO.mCustomPosition.z = 0.0f;
        playerSO.mNameTag = sceneobject_constants::PLAYER_SCENE_OBJECT_NAME;
        playerSO.mObjectFamilyTypeName = strutils::StringId("player");
        
        ObjectTypeDefinitionRepository::GetInstance().LoadObjectTypeDefinition(playerSO.mObjectFamilyTypeName);
        
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
        wallShape.SetAsBox(1.0f, worldCam.GetCameraLenseHeight()/2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::WALLS_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        so.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::WALLS_TEXTURE_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        AddSceneObject(std::move(so));
    }
    
    // R_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(worldCam.GetCameraLenseWidth()/2, 0.0f);
        
        b2Body* wallBody = mBox2dWorld.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, worldCam.GetCameraLenseHeight()/2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = physics_constants::WALLS_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        so.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::WALLS_TEXTURE_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        AddSceneObject(std::move(so));
    }

    // B_WALL
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
        fixtureDef.filter.categoryBits = physics_constants::WALLS_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        so.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::WALLS_TEXTURE_FILE_NAME);
        so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        AddSceneObject(std::move(so));
    }
    
    // Joystick
    {
        SceneObject joystickSO;
        joystickSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        joystickSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::JOYSTICK_TEXTURE_FILE_NAME);
        joystickSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        joystickSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickSO.mCustomScale = gameobject_constants::JOYSTICK_SCALE;
        joystickSO.mNameTag = sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME;
        joystickSO.mInvisible = true;
        AddSceneObject(std::move(joystickSO));
    }
    
    // Joystick Bounds
    {
        SceneObject joystickBoundsSO;
        joystickBoundsSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        joystickBoundsSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::JOYSTICK_BOUNDS_TEXTURE_FILE_NAME);
        joystickBoundsSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        joystickBoundsSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickBoundsSO.mCustomScale = gameobject_constants::JOYSTICK_BOUNDS_SCALE;
        joystickBoundsSO.mNameTag = sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME;
        joystickBoundsSO.mInvisible = true;
        AddSceneObject(std::move(joystickBoundsSO));
    }
}

///------------------------------------------------------------------------------------------------
