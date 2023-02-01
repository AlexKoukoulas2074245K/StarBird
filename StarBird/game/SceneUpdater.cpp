///------------------------------------------------------------------------------------------------
///  SceneUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "InputContext.h"
#include "PhysicsConstants.h"
#include "Scene.h"
#include "SceneUpdater.h"
#include "SceneObjectConstants.h"
#include "GameObjectConstants.h"
#include "GameSingletons.h"
#include "ObjectTypeDefinitionRepository.h"
#include "../utils/Logging.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/TextureResource.h"

#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

SceneUpdater::SceneUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
{
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::SetLevelProperties(LevelDefinition&& levelDef)
{
    mLevel = levelDef;
    
    mFlows.emplace_back([&]()
    {
        auto playerOpt = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerOpt)
        {
            SceneObject so;
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.position = playerOpt->get().mBody->GetWorldCenter();
            bodyDef.bullet =  true;
            b2Body* body = mBox2dWorld.CreateBody(&bodyDef);
            body->SetLinearVelocity(b2Vec2(0.0f, 8.0f));
            b2PolygonShape dynamicBox;
            
            auto& resService = resources::ResourceLoadingService::GetInstance();
            auto bulletTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::BULLET_TEXTURE_FILE_NAME);
            auto& bulletTexture = resService.GetResource<resources::TextureResource>(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::BULLET_TEXTURE_FILE_NAME);
            
            float textureAspect = bulletTexture.GetDimensions().x/bulletTexture.GetDimensions().y;
            dynamicBox.SetAsBox(0.25f, 0.25f/textureAspect);
            
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &dynamicBox;
            fixtureDef.density = 0.1f;
            fixtureDef.friction = 0.0f;
            fixtureDef.restitution = 0.0f;
            fixtureDef.filter.categoryBits = physics_constants::PLAYER_BULLET_CATEGORY_BIT;
            fixtureDef.filter.maskBits &= (~physics_constants::PLAYER_CATEGORY_BIT);
            fixtureDef.filter.maskBits &= (~physics_constants::PLAYER_BULLET_CATEGORY_BIT);
            
            body->CreateFixture(&fixtureDef);
            
            so.mBody = body;
            so.mCustomPosition.z = -0.5f;
            so.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
            so.mTextureResourceId = bulletTextureResourceId;
            so.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
            so.mSceneObjectType = SceneObjectType::WorldGameObject;
            so.mNameTag.fromAddress(so.mBody);

            mScene.AddSceneObject(std::move(so));
        }
    }, 300.0f, RepeatableFlow::RepeatPolicy::REPEAT);
    
    class ContactListener : public b2ContactListener
    {
    public:
        ContactListener(Scene& scene): mScene(scene) {}
        
        void BeginContact(b2Contact* contact) override
        {
            const auto catA = contact->GetFixtureA()->GetFilterData().categoryBits;
            const auto catB = contact->GetFixtureB()->GetFilterData().categoryBits;
            
            if (catA == physics_constants::ENEMY_CATEGORY_BIT && catB == physics_constants::PLAYER_BULLET_CATEGORY_BIT)
            {
                strutils::StringId bodyAddressTag;
                bodyAddressTag.fromAddress(contact->GetFixtureA()->GetBody());
                auto sceneObject = mScene.GetSceneObject(bodyAddressTag);
                
                if (sceneObject)
                {
                    if (sceneObject->get().mHealth <= 1)
                    {
                        mScene.RemoveAllSceneObjectsWithNameTag(bodyAddressTag);
                    }
                    else
                    {
                        sceneObject->get().mHealth--;
                    }
                }
                
                strutils::StringId bulletAddressTag;
                bulletAddressTag.fromAddress(contact->GetFixtureB()->GetBody());
                mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
            }
            else if (catA == physics_constants::PLAYER_BULLET_CATEGORY_BIT && catB == physics_constants::ENEMY_CATEGORY_BIT)
            {
                strutils::StringId bodyAddressTag;
                bodyAddressTag.fromAddress(contact->GetFixtureB()->GetBody());
                auto sceneObject = mScene.GetSceneObject(bodyAddressTag);
                
                if (sceneObject)
                {
                    if (sceneObject->get().mHealth <= 1)
                    {
                        mScene.RemoveAllSceneObjectsWithNameTag(bodyAddressTag);
                    }
                    else
                    {
                        sceneObject->get().mHealth--;
                    }
                }
                
                strutils::StringId bulletAddressTag;
                bulletAddressTag.fromAddress(contact->GetFixtureA()->GetBody());
                mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
            }
        }
        
    private:
        Scene& mScene;
    };
    
    static ContactListener cl(mScene);
    mBox2dWorld.SetContactListener(&cl);
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMilis)
{
    auto playerSO = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
    auto bgSO = mScene.GetSceneObject(sceneobject_constants::BACKGROUND_SCENE_OBJECT_NAME);
    
    for (auto& sceneObject: sceneObjects)
    {
        // Check if this scene object has a respective family object definition
        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject.mObjectFamilyTypeName);
        if (sceneObjectTypeDefOpt)
        {
            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
            switch (sceneObjectTypeDef.mMovementControllerPattern)
            {
                case MovementControllerPattern::CUSTOM_VELOCITY:
                {
                    sceneObject.mBody->SetLinearVelocity(b2Vec2(sceneObjectTypeDef.mCustomLinearVelocity.x, sceneObjectTypeDef.mCustomLinearVelocity.y));
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
                            toAttractionPoint.x *= dtMilis * sceneObjectTypeDef.mSpeed;
                            toAttractionPoint.y *= dtMilis * sceneObjectTypeDef.mSpeed;
                            sceneObject.mBody->ApplyForceToCenter(toAttractionPoint, true);
                        }
                    }
                } break;
                    
                case MovementControllerPattern::INPUT_CONTROLLED:
                {
                    UpdateInputControlledSceneObject(sceneObject, sceneObjectTypeDef, dtMilis);
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
    
    for (auto& flow: mFlows)
    {
        flow.update(dtMilis);
    }
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
{
    const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
    assert(camOpt);
    const auto& guiCamera = camOpt->get();
    
    auto joystickSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSO = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    const auto& inputContext = GameSingletons::GetInputContext();
    
    switch (inputContext.mLastEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mCustomPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
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
                auto motionVec = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix()) - joystickBoundsSO->get().mCustomPosition;

                glm::vec3 norm = glm::normalize(motionVec);
                if (glm::length(motionVec) > glm::length(norm))
                {
                    motionVec = norm;
                }
                
                joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition + motionVec;
                joystickSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_Z;
                
                motionVec.x *= sceneObjectTypeDef.mSpeed * dtMilis;
                motionVec.y *= sceneObjectTypeDef.mSpeed * dtMilis;

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
