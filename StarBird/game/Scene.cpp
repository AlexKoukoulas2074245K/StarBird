///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Scene.h"
#include "dataloaders/GameObjectDefinitionLoader.h"
#include "dataloaders/LevelDataLoader.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/TextureResource.h"

#include <algorithm>
#include <optional>
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

Scene::Scene(b2World& world)
    : mWorld(world)
    , mSceneUpdater(*this)
    , mPreFirstUpdate(true)
{
    auto* window = SDL_GL_GetCurrentWindow();
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    mSceneObjectTypeToCamera[SceneObjectType::GameObject] = Camera(windowWidth, windowHeight, 30.0f);
    mSceneObjectTypeToCamera[SceneObjectType::GUIObject] = Camera(windowWidth, windowHeight, 30.0f);
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
    
    GameObjectDefinitionLoader goDefinitionLoader;
    std::unordered_map<strutils::StringId, GameObjectDefinition, strutils::StringIdHasher> enemyTypesToDefinitions;
    
    for (auto& enemyType: levelDef.mEnemyTypes)
    {
        enemyTypesToDefinitions[enemyType] = goDefinitionLoader.LoadGameObjectDefinition(enemyType.GetString());
    }
    enemyTypesToDefinitions[strutils::StringId("player")] = goDefinitionLoader.LoadGameObjectDefinition("player");
    
    for (auto& enemy: levelDef.mWaves[0].mEnemies)
    {
        const auto& enemyDef = enemyTypesToDefinitions.at(enemy.mGameObjectEnemyType);
        
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(enemy.mPosition.x, enemy.mPosition.y);
        b2Body* body = mWorld.CreateBody(&bodyDef);
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
        so.mSceneObjectType = SceneObjectType::GameObject;
        so.mCustomPosition.z = 0.0f;
        
        strutils::StringId nameTag;
        nameTag.fromAddress(so.mBody);
        
        so.mNameTag = nameTag;
        AddSceneObject(std::move(so));
    }
    
    mSceneUpdater.SetLevelProperties(std::move(levelDef), std::move(enemyTypesToDefinitions));
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateScene(const float dtMilis, const InputContext& inputContext)
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
            mWorld.DestroyBody(iter->mBody);
            auto sizeBefore = mSceneObjects.size();
            mSceneObjects.erase(iter);
            auto sizeNow = mSceneObjects.size();
            assert(sizeBefore = sizeNow + 1);
        }
    }
    mNameTagsOfSceneObjectsToRemove.clear();
    
    mSceneObjects.insert(mSceneObjects.end(), mSceneObjectsToAdd.begin(), mSceneObjectsToAdd.end());
    mSceneObjectsToAdd.clear();
    
    mSceneUpdater.Update(mSceneObjects, mSceneObjectTypeToCamera, dtMilis, inputContext);
}

///------------------------------------------------------------------------------------------------

void Scene::RenderScene()
{
    mSceneRenderer.Render(mSceneObjects, mSceneObjectTypeToCamera);
}

///------------------------------------------------------------------------------------------------
