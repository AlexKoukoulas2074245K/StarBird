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
#include "../resloading/ShaderResource.h"
#include "../resloading/TextureResource.h"

#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

SceneUpdater::SceneUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mAllowInputControl(false)
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
            body->SetLinearVelocity(b2Vec2(0.0f, 16.0f));
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
            so.mUseBodyForRendering = true;
            
            mScene.AddSceneObject(std::move(so));
        }
    }, 300.0f, RepeatableFlow::RepeatPolicy::REPEAT);
    
    class ContactListener : public b2ContactListener
    {
    public:
        ContactListener(Scene& scene, SceneUpdater& sceneUpdater, std::vector<RepeatableFlow>& flows)
            : mScene(scene)
            , mSceneUpdater(sceneUpdater)
            , mFlows(flows) {}
        
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
                        sceneObject->get().mStateName = strutils::StringId("dying");
                        sceneObject->get().mUseBodyForRendering = false;
                        sceneObject->get().mCustomPosition.x = contact->GetFixtureA()->GetBody()->GetWorldCenter().x;
                        sceneObject->get().mCustomPosition.y = contact->GetFixtureA()->GetBody()->GetWorldCenter().y;
                        
                        auto filter = contact->GetFixtureA()->GetFilterData();
                        filter.maskBits = 0;
                        contact->GetFixtureA()->SetFilterData(filter);
                        
                        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject->get().mObjectFamilyTypeName);
                        if (sceneObjectTypeDefOpt)
                        {
                            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
                            
                            mSceneUpdater.UpdateAnimation(sceneObject->get(), sceneObjectTypeDef, 0.0f);
                            
                            const auto& currentAnim = sceneObjectTypeDef.mAnimations.at(sceneObject->get().mStateName);
                            
                            mFlows.emplace_back([=]()
                            {
                                mScene.RemoveAllSceneObjectsWithNameTag(bodyAddressTag);
                            }, currentAnim.mDuration, RepeatableFlow::RepeatPolicy::ONCE);
                        }
                    }
                    else
                    {
                        sceneObject->get().mHealth--;
                    }
                }
                
                
                auto filter = contact->GetFixtureB()->GetFilterData();
                filter.maskBits = 0;
                contact->GetFixtureB()->SetFilterData(filter);
                
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
                        sceneObject->get().mStateName = strutils::StringId("dying");
                        sceneObject->get().mUseBodyForRendering = false;
                        sceneObject->get().mCustomPosition.x = contact->GetFixtureB()->GetBody()->GetWorldCenter().x;
                        sceneObject->get().mCustomPosition.y = contact->GetFixtureB()->GetBody()->GetWorldCenter().y;
                        
                        auto filter = contact->GetFixtureB()->GetFilterData();
                        filter.maskBits = 0;
                        contact->GetFixtureB()->SetFilterData(filter);
                        
                        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject->get().mObjectFamilyTypeName);
                        if (sceneObjectTypeDefOpt)
                        {
                            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
                            
                            mSceneUpdater.UpdateAnimation(sceneObject->get(), sceneObjectTypeDef, 0.0f);
                            
                            const auto& currentAnim = sceneObjectTypeDef.mAnimations.at(sceneObject->get().mStateName);
                            mFlows.emplace_back([=]()
                            {
                                mScene.RemoveAllSceneObjectsWithNameTag(bodyAddressTag);
                            }, currentAnim.mDuration, RepeatableFlow::RepeatPolicy::ONCE);
                        }
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
            else if (catA == physics_constants::PLAYER_BULLET_CATEGORY_BIT && catB == physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT)
            {
                strutils::StringId bulletAddressTag;
                bulletAddressTag.fromAddress(contact->GetFixtureA()->GetBody());
                mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
            }
            else if (catB == physics_constants::PLAYER_BULLET_CATEGORY_BIT && catA == physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT)
            {
                strutils::StringId bulletAddressTag;
                bulletAddressTag.fromAddress(contact->GetFixtureB()->GetBody());
                mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
            }
        }
        
    private:
        Scene& mScene;
        SceneUpdater& mSceneUpdater;
        std::vector<RepeatableFlow>& mFlows;
    };
    
    static ContactListener cl(mScene, *this, mFlows);
    mBox2dWorld.SetContactListener(&cl);
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMilis)
{
    auto playerSO = mScene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME);
    auto bgSO = mScene.GetSceneObject(sceneobject_constants::BACKGROUND_SCENE_OBJECT_NAME);
    
    for (auto& sceneObject: sceneObjects)
    {
        sceneObject.mShaderBoolUniformValues[sceneobject_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = false;
        
        // Check if this scene object has a respective family object definition
        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject.mObjectFamilyTypeName);
        if (sceneObjectTypeDefOpt)
        {
            // Update movement
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
            
            // Update animation
            UpdateAnimation(sceneObject, sceneObjectTypeDef, dtMilis);
        }
    }
    
    static float msAccum = 0.0f;
    msAccum -= dtMilis/4000.0f;
    
    if (bgSO)
    {
       bgSO->get().mShaderFloatUniformValues[sceneobject_constants::TEXTURE_OFFSET_UNIFORM_NAME] = msAccum;
    }
    
    for (auto& flow: mFlows)
    {
        flow.Update(dtMilis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
}

///------------------------------------------------------------------------------------------------

void SceneUpdater::UpdateAnimation(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
{
    const auto& currentAnim = sceneObjectTypeDef.mAnimations.at(sceneObject.mStateName);
    const auto& currentTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(currentAnim.mTextureResourceId);

    if (currentTexture.GetSheetMetadata())
    {
        const auto& sheetMetaDataCurrentRow = currentTexture.GetSheetMetadata()->mRowMetadata[currentAnim.mTextureSheetRow];
        const auto animFrameTime = currentAnim.mDuration/sheetMetaDataCurrentRow.mColMetadata.size();
        
        if (currentAnim.mDuration > 0.0f)
        {
            sceneObject.mAnimationTime += dtMilis;
            if (sceneObject.mAnimationTime >= animFrameTime)
            {
                sceneObject.mAnimationTime = 0.0f;
                sceneObject.mAnimationIndex = (sceneObject.mAnimationIndex + 1) % sheetMetaDataCurrentRow.mColMetadata.size();
            }
        }
        sceneObject.mShaderBoolUniformValues[sceneobject_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MIN_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).minU;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MIN_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).minV;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MAX_U_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).maxU;
        sceneObject.mShaderFloatUniformValues[sceneobject_constants::MAX_V_UNIFORM_NAME] = sheetMetaDataCurrentRow.mColMetadata.at(sceneObject.mAnimationIndex).maxV;
        
        sceneObject.mCustomScale = glm::vec3(currentAnim.mScale, currentAnim.mScale, 1.0f);
    }
    
    sceneObject.mTextureResourceId = currentAnim.mTextureResourceId;
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
    
    switch (inputContext.mEventType)
    {
        case SDL_FINGERDOWN:
        {
             if (joystickBoundsSO && joystickSO)
             {
                 joystickBoundsSO->get().mCustomPosition = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), inputContext.mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
                 joystickBoundsSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_Z;
                 
                 joystickSO->get().mCustomPosition = joystickBoundsSO->get().mCustomPosition;
                 joystickSO->get().mCustomPosition.z = gameobject_constants::JOYSTICK_BOUNDS_Z;

                 mAllowInputControl = true;
             }
        } break;
            
        case SDL_FINGERUP:
        {
            sceneObject.mBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
        } break;
            
        case SDL_FINGERMOTION:
        {
            if (joystickBoundsSO && joystickSO && mAllowInputControl)
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
    
    if (joystickSO && joystickBoundsSO && mAllowInputControl)
    {
       joystickSO->get().mInvisible = inputContext.mEventType == SDL_FINGERUP;
       joystickBoundsSO->get().mInvisible = inputContext.mEventType == SDL_FINGERUP;
    }
}

///------------------------------------------------------------------------------------------------
