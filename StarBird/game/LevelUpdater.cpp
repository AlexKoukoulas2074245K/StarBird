///------------------------------------------------------------------------------------------------
///  LevelUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameObjectConstants.h"
#include "GameSingletons.h"
#include "InputContext.h"
#include "LevelUpdater.h"
#include "ObjectTypeDefinitionRepository.h"
#include "PhysicsConstants.h"
#include "PhysicsCollisionListener.h"
#include "Scene.h"
#include "SceneObjectConstants.h"

#include "../utils/Logging.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/ShaderResource.h"
#include "../resloading/TextureResource.h"

#include <algorithm>
#include <Box2D/Box2D.h>
#include <map>

///------------------------------------------------------------------------------------------------

LevelUpdater::LevelUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mBox2dWorld(box2dWorld)
    , mCurrentWaveNumber(0)
    , mState(LevelState::WAVE_INTRO)
    , mAllowInputControl(false)
{
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::InitLevel(LevelDefinition&& levelDef)
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
    }, 300.0f, RepeatableFlow::RepeatPolicy::REPEAT, gameobject_constants::PLAYER_BULLET_FLOW_NAME);
    
    static PhysicsCollisionListener collisionListener;
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_CATEGORY_BIT, physics_constants::PLAYER_BULLET_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        strutils::StringId enemyBodyAddressTag;
        enemyBodyAddressTag.fromAddress(firstBody);
        auto enemySceneObjectOpt = mScene.GetSceneObject(enemyBodyAddressTag);
        
        if (enemySceneObjectOpt)
        {
            auto& enemySO = enemySceneObjectOpt->get();
            
            if (enemySO.mHealth <= 1)
            {
                enemySO.mStateName = strutils::StringId("dying");
                enemySO.mUseBodyForRendering = false;
                enemySO.mCustomPosition.x = firstBody->GetWorldCenter().x;
                enemySO.mCustomPosition.y = firstBody->GetWorldCenter().y;
                
                auto filter = firstBody->GetFixtureList()[0].GetFilterData();
                filter.maskBits = 0;
                firstBody->GetFixtureList()[0].SetFilterData(filter);
                
                auto enemySceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(enemySO.mObjectFamilyTypeName);
                if (enemySceneObjectTypeDefOpt)
                {
                    auto& enemySOTypeDef = enemySceneObjectTypeDefOpt->get();
                    
                    UpdateAnimation(enemySO, enemySOTypeDef, 0.0f);
                    
                    const auto& currentAnim = enemySOTypeDef.mAnimations.at(enemySO.mStateName);
                    
                    mFlows.emplace_back([=]()
                    {
                        mWaveEnemies.erase(enemyBodyAddressTag);
                        mScene.RemoveAllSceneObjectsWithNameTag(enemyBodyAddressTag);
                    }, currentAnim.mDuration, RepeatableFlow::RepeatPolicy::ONCE);
                }
            }
            else
            {
                enemySO.mHealth--;
            }
        }
        
        // Erase bullet collision mask so that it doesn't also contribute to other
        // enemy damage until it is removed from b2World
        auto bulletFilter = secondBody->GetFixtureList()[0].GetFilterData();
        bulletFilter.maskBits = 0;
        secondBody->GetFixtureList()[0].SetFilterData(bulletFilter);
        
        strutils::StringId bulletAddressTag;
        bulletAddressTag.fromAddress(secondBody);
        
        mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
    });
    
    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::PLAYER_BULLET_CATEGORY_BIT, physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        strutils::StringId bulletAddressTag;
        bulletAddressTag.fromAddress(firstBody);
        mScene.RemoveAllSceneObjectsWithNameTag(bulletAddressTag);
    });

    collisionListener.RegisterCollisionCallback(UnorderedCollisionCategoryPair(physics_constants::ENEMY_CATEGORY_BIT, physics_constants::ENEMY_ONLY_WALL_CATEGORY_BIT), [&](b2Body* firstBody, b2Body* secondBody)
    {
        strutils::StringId enemyAddressTag;
        enemyAddressTag.fromAddress(firstBody);
        mScene.RemoveAllSceneObjectsWithNameTag(enemyAddressTag);
        mWaveEnemies.erase(enemyAddressTag);
    });

    mBox2dWorld.SetContactListener(&collisionListener);
    
    mState = LevelState::WAVE_INTRO;
    mCurrentWaveNumber = 0;
    CreateWaveIntroText();
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::Update(std::vector<SceneObject>& sceneObjects, const float dtMilis)
{
    auto stateMachineUpdateResult = UpdateStateMachine(dtMilis);
    if (stateMachineUpdateResult == StateMachineUpdateResult::BLOCK_UPDATE)
    {
        OnBlockedUpdate();
        return;
    }
    
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
    msAccum -= dtMilis * gameobject_constants::BACKGROUND_SPEED;
    
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

size_t LevelUpdater::GetWaveEnemyCount() const
{
    return mWaveEnemies.size();
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<RepeatableFlow>> LevelUpdater::GetFlow(const strutils::StringId& flowName)
{
    auto findIter = std::find_if(mFlows.begin(), mFlows.end(), [&](const RepeatableFlow& flow)
    {
        return flow.GetName() == flowName;
    });
    
    if (findIter != mFlows.end())
    {
        return std::optional<std::reference_wrapper<RepeatableFlow>>{*findIter};
    }
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

LevelUpdater::StateMachineUpdateResult LevelUpdater::UpdateStateMachine(const float dtMilis)
{
    static float tween = 0.0f;
    
    switch (mState)
    {
        case LevelState::WAVE_INTRO:
        {
            auto waveTextIntroSoOpt = mScene.GetSceneObject(sceneobject_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME);
            auto waveTextIntroFlowOpt = GetFlow(gameobject_constants::WAVE_INTRO_FLOW_NAME);
            
            if (waveTextIntroSoOpt && waveTextIntroFlowOpt)
            {
                auto& waveTextIntroSo = waveTextIntroSoOpt->get();
                auto& waveTextIntroFlow = waveTextIntroFlowOpt->get();
                
                const auto ticksLeft = waveTextIntroFlow.GetTicksLeft();
                const auto halfDuration = waveTextIntroFlow.GetDuration()/2;
                if (ticksLeft > halfDuration)
                {
                    waveTextIntroSo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f - (ticksLeft - halfDuration)/halfDuration;
                }
                else
                {
                    waveTextIntroSo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = ticksLeft/halfDuration;
                }
            }
        } break;
            
        case LevelState::FIGHTING_WAVE:
        {
            if (mWaveEnemies.size() == 0)
            {
                mState = LevelState::UPGRADE_OVERLAY_IN;
                CreateUpgradeSceneObjects();
                tween = 0.0f;
            }
        } break;
        
        case LevelState::UPGRADE_OVERLAY_IN:
        {
            auto upgradeOverlaySoOpt = mScene.GetSceneObject(sceneobject_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME);
            if (upgradeOverlaySoOpt)
            {
                auto& upgradeOverlaySo = upgradeOverlaySoOpt->get();
                auto& upgradeOverlayAlpha = upgradeOverlaySo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME];
                upgradeOverlayAlpha += dtMilis * gameobject_constants::OVERLAY_DARKENING_SPEED;
                if (upgradeOverlayAlpha >= gameobject_constants::UPGRADE_OVERLAY_MAX_ALPHA)
                {
                    upgradeOverlayAlpha = gameobject_constants::UPGRADE_OVERLAY_MAX_ALPHA;
                    mState = LevelState::UPGRADE;
                }
            }
            return StateMachineUpdateResult::BLOCK_UPDATE;
        } break;
            
        case LevelState::UPGRADE:
        {
            tween += dtMilis * gameobject_constants::UPGRADE_MOVEMENT_SPEED;
            float perc = math::Min(1.0f, math::TweenValue(tween, math::QuartFunction, math::TweeningMode::EASE_IN));
            
            auto leftUpgradeContainerSoOpt = mScene.GetSceneObject(sceneobject_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
            auto rightUpgradeContainerSoOpt = mScene.GetSceneObject(sceneobject_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME);
            
            if (leftUpgradeContainerSoOpt)
            {
                leftUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::LEFT_UPGRADE_CONTAINER_INIT_POS.x + perc * gameobject_constants::LEFT_UPGRADE_CONTAINER_TARGET_POS.x;
            }
            
            if (rightUpgradeContainerSoOpt)
            {
                rightUpgradeContainerSoOpt->get().mCustomPosition.x = (1.0f - perc) * gameobject_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS.x + perc * gameobject_constants::RIGHT_UPGRADE_CONTAINER_TARGET_POS.x;
            }
            
//            mCurrentWaveNumber++;
//            CreateWaveIntroText();
            
            return StateMachineUpdateResult::BLOCK_UPDATE;
        } break;
            
        case LevelState::UPGRADE_OVERLAY_OUT:
        {
            return StateMachineUpdateResult::BLOCK_UPDATE;
        } break;
            
        default: break;
    }
    
    return StateMachineUpdateResult::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::UpdateAnimation(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
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

void LevelUpdater::UpdateInputControlledSceneObject(SceneObject& sceneObject, const ObjectTypeDefinition& sceneObjectTypeDef, const float dtMilis)
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

void LevelUpdater::OnBlockedUpdate()
{
    auto joystickSoOpt = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME);
    auto joystickBoundsSOopt = mScene.GetSceneObject(sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME);
    
    mScene.FreezeAllPhysicsBodies();
    
    if (joystickSoOpt)
    {
        joystickSoOpt->get().mInvisible = true;
    }
    
    if (joystickBoundsSOopt)
    {
        joystickBoundsSOopt->get().mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateWaveIntroText()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    SceneObject waveTextSO;
    waveTextSO.mCustomPosition = gameobject_constants::WAVE_INTRO_TEXT_INIT_POS;
    waveTextSO.mCustomScale = gameobject_constants::WAVE_INTRO_TEXT_SCALE;
    waveTextSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
    waveTextSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
    waveTextSO.mTextureResourceId = FontRepository::GetInstance().GetFont(strutils::StringId("font"))->get().mFontTextureResourceId;
    waveTextSO.mFontName = strutils::StringId("font");
    waveTextSO.mSceneObjectType = SceneObjectType::GUIObject;
    waveTextSO.mNameTag = sceneobject_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME;
    waveTextSO.mText = "WAVE " + std::to_string(mCurrentWaveNumber + 1);
    waveTextSO.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene.AddSceneObject(std::move(waveTextSO));
    
    mFlows.emplace_back([&]()
    {
        mScene.RemoveAllSceneObjectsWithNameTag(sceneobject_constants::WAVE_INTRO_TEXT_SCNE_OBJECT_NAME);
        mState = LevelState::FIGHTING_WAVE;
        CreateWave();
    }, gameobject_constants::WAVE_INTRO_DURATION_MILIS, RepeatableFlow::RepeatPolicy::ONCE, gameobject_constants::WAVE_INTRO_FLOW_NAME);
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateWave()
{
    auto& objectTypeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    for (const auto& enemy: mLevel.mWaves[mCurrentWaveNumber].mEnemies)
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
        auto& enemyTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(enemyDef.mAnimations.at(sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId);
        
        float textureAspect = enemyTexture.GetDimensions().x/enemyTexture.GetDimensions().y;
        dynamicBox.SetAsBox(enemyDef.mSize, enemyDef.mSize/textureAspect);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = enemyDef.mDensity;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter = enemyDef.mContactFilter;
        fixtureDef.filter.maskBits &= (~physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
        fixtureDef.filter.maskBits &= (~physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT);
        body->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mObjectFamilyTypeName = enemy.mGameObjectEnemyType;
        so.mBody = body;
        so.mHealth = enemyDef.mHealth;
        
        so.mShaderResourceId = enemyDef.mShaderResourceId;
        so.mTextureResourceId = enemyDef.mAnimations.at(sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE).mTextureResourceId;
        so.mMeshResourceId = enemyDef.mMeshResourceId;
        so.mSceneObjectType = SceneObjectType::WorldGameObject;
        so.mCustomPosition.z = 0.0f;
        so.mUseBodyForRendering = true;
        
        strutils::StringId nameTag;
        nameTag.fromAddress(so.mBody);
        
        mWaveEnemies.insert(nameTag);
        
        so.mNameTag = nameTag;
        mScene.AddSceneObject(std::move(so));
    }
}

///------------------------------------------------------------------------------------------------

void LevelUpdater::CreateUpgradeSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::CUSTOM_ALPHA_SHADER_FILE_NAME);
        overlaySo.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::UPGRADE_OVERLAY_TEXTURE_FILE_NAME);
        overlaySo.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mCustomScale = gameobject_constants::UPGRADE_OVERLAY_SCALE;
        overlaySo.mCustomPosition = gameobject_constants::UPGRADE_OVERLAY_POSITION;
        overlaySo.mNameTag = sceneobject_constants::UPGRADE_OVERLAY_SCENE_OBJECT_NAME;
        overlaySo.mShaderFloatUniformValues[sceneobject_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mScene.AddSceneObject(std::move(overlaySo));
    }
    
    // Left Upgrade Container
    {
        SceneObject leftUpgradeSO;
        leftUpgradeSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        leftUpgradeSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME);
        leftUpgradeSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        leftUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        leftUpgradeSO.mCustomScale = gameobject_constants::LEFT_UPGRADE_CONTAINER_SCALE;
        leftUpgradeSO.mCustomPosition = gameobject_constants::LEFT_UPGRADE_CONTAINER_INIT_POS;
        leftUpgradeSO.mNameTag = sceneobject_constants::LEFT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(leftUpgradeSO));
    }
    
    // Right Upgrade Container
    {
        SceneObject rightUpgradeSO;
        rightUpgradeSO.mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneobject_constants::BASIC_SHADER_FILE_NAME);
        rightUpgradeSO.mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneobject_constants::UPGRADE_CONTAINER_TEXTURE_FILE_NAME);
        rightUpgradeSO.mMeshResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + sceneobject_constants::QUAD_MESH_FILE_NAME);
        rightUpgradeSO.mSceneObjectType = SceneObjectType::GUIObject;
        rightUpgradeSO.mCustomScale = gameobject_constants::RIGHT_UPGRADE_CONTAINER_SCALE;
        rightUpgradeSO.mCustomPosition = gameobject_constants::RIGHT_UPGRADE_CONTAINER_INIT_POS;
        rightUpgradeSO.mNameTag = sceneobject_constants::RIGHT_UPGRADE_CONTAINER_SCENE_OBJECT_NAME;
        mScene.AddSceneObject(std::move(rightUpgradeSO));
    }
}

///------------------------------------------------------------------------------------------------
