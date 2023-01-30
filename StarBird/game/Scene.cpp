///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Scene.h"
#include <algorithm>
#include <optional>

///------------------------------------------------------------------------------------------------

Scene::Scene()
{
    
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
    mSceneObjectsToAdd.emplace_back(sceneObject);
}

///------------------------------------------------------------------------------------------------

void Scene::RemoveAllSceneObjectsWithNameTag(const strutils::StringId& nameTag)
{
    mNameTagsOfSceneObjectsToRemove.push_back(nameTag);
}

///------------------------------------------------------------------------------------------------

void Scene::UpdateScene()
{
    for (const auto& nameTag: mNameTagsOfSceneObjectsToRemove)
    {
        auto sizeBefore = mSceneObjects.size();
        mSceneObjects.erase(std::remove_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const SceneObject& so)
        {
            return so.mNameTag == nameTag;
        }), mSceneObjects.end());
        auto sizeNow = mSceneObjects.size();
        assert(sizeBefore = sizeNow + 1);
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
