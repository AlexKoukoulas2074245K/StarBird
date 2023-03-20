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
#include "MapUpdater.h"
#include "LevelUpdater.h"
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
    , mSceneRenderer(mBox2dWorld)
    , mPreFirstUpdate(true)
{
    FontRepository::GetInstance().LoadFont(scene_object_constants::DEFAULT_FONT_NAME);
    GameSingletons::SetCameraForSceneObjectType(SceneObjectType::WorldGameObject, Camera());
    GameSingletons::SetCameraForSceneObjectType(SceneObjectType::GUIObject, Camera());
    mSceneUpdater = std::make_unique<MapUpdater>(*this);
}

///------------------------------------------------------------------------------------------------

Scene::~Scene()
{
    
}

///------------------------------------------------------------------------------------------------

std::string Scene::GetSceneStateDescription() const
{
    return "SOs: " + std::to_string(mSceneObjects.size()) + " bodies: " + std::to_string(mBox2dWorld.GetBodyCount()) + " enemies: " + mSceneUpdater->GetDescription();
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
    
    mSceneUpdater = std::make_unique<LevelUpdater>(*this, mBox2dWorld, std::move(levelDef));
}

///------------------------------------------------------------------------------------------------

void Scene::OnAppStateChange(Uint32 event)
{
    mSceneUpdater->OnAppStateChange(event);
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateScene(const float dtMillis)
{
    mPreFirstUpdate = false;
    
    mSceneUpdater->Update(mSceneObjects, dtMillis * GameSingletons::GetGameSpeedMultiplier());
    
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
    mSceneUpdater->OpenDebugConsole();
}
#endif

///------------------------------------------------------------------------------------------------

