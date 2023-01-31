///------------------------------------------------------------------------------------------------
///  SceneUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Scene.h"
#include "SceneUpdater.h"
#include "SceneObjectConstants.h"
#include "GameObjectConstants.h"

///------------------------------------------------------------------------------------------------

SceneUpdater::SceneUpdater(Scene& scene)
    : mScene(scene)
{
    
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::SetLevelProperties(LevelDefinition&& levelDef, std::unordered_map<strutils::StringId, GameObjectDefinition, strutils::StringIdHasher>&& enemyTypesToDefinitions)
{
    mLevel = levelDef;
    mEnemyTypesToDefinitions = enemyTypesToDefinitions;
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::Update(std::vector<SceneObject>& sceneObjects, const std::unordered_map<SceneObjectType, Camera>& sceneObjectTypeToCamera, const float dtMilis, const InputContext& inputContext)
{
    auto playerSO = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
    auto bgSO = mScene.GetSceneObject(sceneobject_constants::BACKGROUND_SCENE_OBJECT_NAME);
    
    for (auto& sceneObject: sceneObjects)
    {
        // Check if this scene object has a respective family object definition
        auto enemyTypeDefIter = mEnemyTypesToDefinitions.find(sceneObject.mObjectFamilyTypeName);
        if (enemyTypeDefIter != mEnemyTypesToDefinitions.end())
        {
            switch (enemyTypeDefIter->second.mMovementControllerPattern)
            {
                case MovementControllerPattern::CUSTOM_VELOCITY:
                {
                    sceneObject.mBody->SetLinearVelocity(b2Vec2(enemyTypeDefIter->second.mCustomLinearVelocity.x, enemyTypeDefIter->second.mCustomLinearVelocity.y));
                } break;
                    
                case MovementControllerPattern::CHASING_PLAYER:
                {
                    if (playerSO)
                    {
                        b2Vec2 toAttractionPoint = playerSO->get().mBody->GetWorldCenter() - sceneObject.mBody->GetWorldCenter();
                        
                        if (toAttractionPoint.Length() < 0.5f)
                        {
                            sceneObject.mBody->SetAwake(false);
                        }
                        else
                        {
                            toAttractionPoint.Normalize();
                            toAttractionPoint.x *= dtMilis * enemyTypeDefIter->second.mSpeed;
                            toAttractionPoint.y *= dtMilis * enemyTypeDefIter->second.mSpeed;
                            sceneObject.mBody->ApplyForceToCenter(toAttractionPoint, true);
                        }
                    }
                } break;
                    
                case MovementControllerPattern::INPUT_CONTROLLED:
                {
                    UpdateInputControlledSceneObject(sceneObject, enemyTypeDefIter->second, sceneObjectTypeToCamera, dtMilis, inputContext);
                } break;
                    
                default: break;
            }
        }
    }
    
    static float msAccum = 0.0f;
    msAccum -= dtMilis/4000.0f;
    
    if (bgSO)
    {
       bgSO->get().mShaderFloatUniformValues[strutils::StringId("texoffset")] = msAccum;
    }
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, GameObjectDefinition& sceneObjectFamilyDef, const std::unordered_map<SceneObjectType, Camera>& sceneObjectTypeToCamera, const float dtMilis, const InputContext& inputContext)
{
    auto* window = SDL_GL_GetCurrentWindow();
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    auto& guiCam = sceneObjectTypeToCamera.at(SceneObjectType::GUIObject);
    
    auto joystickSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    
    switch (inputContext.mLastEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mCustomPosition = math::ComputeTouchCoordsInWorldSpace(glm::vec2(windowWidth, windowHeight), inputContext.mTouchPos, guiCam.GetViewMatrix(), guiCam.GetProjMatrix());
                 joystickBoundsSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_Z;
                 
                 joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition;
                 joystickSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_BOUNDS_Z;
             }
        } break;
            
        case SDL_FINGERUP:
        {
            sceneObject.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        } break;
            
        case SDL_FINGERMOTION:
        {
            if (joystickBoundsSO && joystickSO)
            {
                auto motionVec = math::ComputeTouchCoordsInWorldSpace(glm::vec2(windowWidth, windowHeight), inputContext.mTouchPos, guiCam.GetViewMatrix(), guiCam.GetProjMatrix()) - joystickBoundsSO->get().mCustomPosition;

                glm::vec3 norm = glm::normalize(motionVec);
                if (glm::length(motionVec) > glm::length(norm))
                {
                    motionVec = norm;
                }
                
                joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition + motionVec;
                joystickSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_Z;
                
                motionVec.x *= sceneObjectFamilyDef.mSpeed * dtMilis;
                motionVec.y *= sceneObjectFamilyDef.mSpeed * dtMilis;

                sceneObject.mBody->SetLinearVelocity(b2Vec2(motionVec.x, motionVec.y));
            }
            
        } break;
            
        default: break;
    }
    
    
    if (joystickSO && joystickBoundsSO)
    {
       joystickSO->get().mInvisible = inputContext.mLastEventType == SDL_FINGERUP;
       joystickBoundsSO->get().mInvisible = inputContext.mLastEventType == SDL_FINGERUP;
    }
}

///------------------------------------------------------------------------------------------------
