///------------------------------------------------------------------------------------------------
///  EventUpdater.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/05/2023
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "EventUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "TextPromptController.h"
#include "states/DebugConsoleGameState.h"
#include "dataloaders/GUISceneLoader.h"
#include "datarepos/FontRepository.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../utils/Logging.h"
#include "../utils/ObjectiveCUtils.h"

#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId EVENT_BACKGROUND_NAME = strutils::StringId("EVENT_BACKGROUND_NAME");

static const std::string EVENT_OPTION_NAME_PREFIX = "EVENT_OPTION_";

static const glm::vec3 BACKGROUND_POSITION = glm::vec3(0.0f, 0.0f, -7.0f);

static const glm::vec3 EVENT_BACKGROUND_POSITION = glm::vec3(0.0f, 7.1f, 1.0f);
static const glm::vec3 EVENT_BACKGROUND_SCALE = glm::vec3(12.2f, 12.2f, 1.0f);

static const glm::vec3 TEXT_PROMPT_POSITION = glm::vec3(0.0f, -1.0f, 1.0f);
static const glm::vec3 TEXT_PROMPT_SCALE = glm::vec3(12.0f, 10.0f, 1.0f);

static const glm::vec3 FULL_SCREEN_OVERLAY_POSITION = glm::vec3(0.0f, 0.0f, -3.0f);
static const glm::vec3 FULL_SCREEN_OVERLAY_SCALE = glm::vec3(200.0f, 200.0f, 1.0f);

static const glm::vec3 EVENT_OPTIONS_FONT_SCALE = glm::vec3(0.007f, 0.007f, 1.0f);
static const glm::vec3 EVENT_OPTIONS_TEXT_INIT_POSITION = glm::vec3(0.0f, -8.5f, 1.0f);

static const int END_STATE_INDEX = 100000;

static const float DROPPED_CRYSTAL_SPEED = 0.0006f;
static const float DROPPED_CRYSTAL_DISTANCE_FACTOR = 24.0f;
static const float DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG = 2.0f;
static const float EVENT_OPTIONS_TEXT_Y_INCREMENT = 1.5f;


///------------------------------------------------------------------------------------------------

EventUpdater::EventUpdater(Scene& scene, b2World& box2dWorld)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mUpgradeUnlockedHandler(scene, box2dWorld)
    , mSelectedEvent(nullptr)
    , mEventProgressionStateIndex(0U)
    , mPreviousEventProgressionStateIndex(0U)
    , mTransitioning(false)
    , mFadeInOptions(false)
    , mEventCompleted(false)
    , mTextPromptController(nullptr)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif
    
    RegisterEvents();
    SelectRandomEvent();
    CreateSceneObjects();
}

///------------------------------------------------------------------------------------------------

EventUpdater::~EventUpdater()
{
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective EventUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mTransitioning)
    {
        return PostStateUpdateDirective::CONTINUE;
    }
    
    // Animate all SOs
    for (auto& sceneObject: sceneObjects)
    {
        // Check if this scene object has a respective family object definition
        auto sceneObjectTypeDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(sceneObject.mObjectFamilyTypeName);
        if (sceneObjectTypeDefOpt && !sceneObject.mCustomDrivenMovement)
        {
            // Update movement
            auto& sceneObjectTypeDef = sceneObjectTypeDefOpt->get();
            switch (sceneObjectTypeDef.mMovementControllerPattern)
            {
                case MovementControllerPattern::CONSTANT_VELOCITY:
                {
                    sceneObject.mBody->SetLinearVelocity(b2Vec2(sceneObjectTypeDef.mConstantLinearVelocity.x, sceneObjectTypeDef.mConstantLinearVelocity.y));
                } break;
                default: break;
            }
        }
        
        if (sceneObject.mAnimation && !sceneObject.mAnimation->VIsPaused())
        {
            sceneObject.mAnimation->VUpdate(dtMillis, sceneObject);
        }
        
        for (auto& extraAnimation: sceneObject.mExtraCompoundingAnimations)
        {
            if (!extraAnimation->VIsPaused())
            {
                extraAnimation->VUpdate(dtMillis, sceneObject);
            }
        }
    }
    
    // Update flows
    for (size_t i = 0; i < mFlows.size(); ++i)
    {
        mFlows[i].Update(dtMillis);
    }
    
    mFlows.erase(std::remove_if(mFlows.begin(), mFlows.end(), [](const RepeatableFlow& flow)
    {
        return !flow.IsRunning();
    }), mFlows.end());
    
    // Event completion wait check
    if (mEventCompleted)
    {
        if (mUpgradeUnlockedHandler.Update(dtMillis) == UpgradeUnlockedHandler::UpgradeAnimationState::FINISHED)
        {
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            mTransitioning = true;
        }
        
        return PostStateUpdateDirective::CONTINUE;
    }
    
    // Debug Console or Popup taking over
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE) return PostStateUpdateDirective::BLOCK_UPDATE;
    
    // Animate text prompt
    if (mTextPromptController)
    {
        mTextPromptController->Update(dtMillis);
    }
    
    if (mFadeInOptions && mUpgradeUnlockedHandler.Update(dtMillis) == UpgradeUnlockedHandler::UpgradeAnimationState::FINISHED)
    {
        auto& inputContext = GameSingletons::GetInputContext();
        auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::GUIObject);
        auto& guiCamera = camOpt->get();
        auto touchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, guiCamera.GetViewMatrix(), guiCamera.GetProjMatrix());
        
        const auto& currentEventOptions = mSelectedEvent->mEventOptions[mEventProgressionStateIndex];
        for (int i = 0; i < currentEventOptions.size(); ++i)
        {
            auto optionName = strutils::StringId(EVENT_OPTION_NAME_PREFIX + std::to_string(i));
            auto optionSoOpt = mScene.GetSceneObject(optionName);
            
            if (optionSoOpt)
            {
                auto& optionSo = optionSoOpt->get();
                optionSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (optionSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    optionSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
                
                // Check for touch
                if (inputContext.mEventType == SDL_FINGERDOWN && scene_object_utils::IsPointInsideSceneObject(optionSo, touchPos))
                {
                    currentEventOptions[i].mSelectionCallback();
                    
                    mPreviousEventProgressionStateIndex = mEventProgressionStateIndex;
                    mEventProgressionStateIndex = currentEventOptions[i].mNextStateIndex;
                    
                    if (mEventProgressionStateIndex > mSelectedEvent->GetStateCount())
                    {
                        mEventCompleted = true;
                    }
                    else
                    {
                        CreateEventSceneObjectsForCurrentState();
                    }
                }
            }
        }
    }

    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void EventUpdater::VOnAppStateChange(Uint32 event)
{
    static bool hasLeftForegroundOnce = false;
    
    switch (event)
    {
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND:
        {
#ifdef DEBUG
            hasLeftForegroundOnce = true;
#endif
        } break;
            
        case SDL_APP_WILLENTERFOREGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
        {
#ifdef DEBUG
            if (hasLeftForegroundOnce)
            {
                VOpenDebugConsole();
            }
#endif
        } break;
    }
}

///------------------------------------------------------------------------------------------------

std::string EventUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

strutils::StringId EventUpdater::VGetStateMachineActiveStateName() const
{
    return mStateMachine.GetActiveStateName();
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void EventUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void EventUpdater::RegisterEvents()
{
    mRegisteredEvents.emplace_back(EventDescription
    (
        {
            "backgrounds/events/0.bmp"
        },

        {
            "You discover a powerful technique that can protect the vessel from a limited amount of enemy projectiles. All crystal reserves  will be depleted in the process of making this shield",
            "You consume all crystals to create a powerful anti-alien shield.",
            "You decide not to expend all your crystals for this shield."
        },

        {
            {
                EventOption("Gain Shield. Loose ALL crystals", 1, [&]()
                {
                    CreateCrystalsTowardTargetPosition(GameSingletons::GetCrystalCount(), EVENT_BACKGROUND_POSITION);
                    GameSingletons::SetCrystalCount(0);
                    mUpgradeUnlockedHandler.OnUpgradeGained(game_constants::PLAYER_SHIELD_UPGRADE_NAME);
                }),
                EventOption("Ignore", 2, [](){}),
            },
            
            {
                EventOption("Leave", END_STATE_INDEX, [](){})
            },
            
            {
                EventOption("Leave", END_STATE_INDEX, [](){})
            }
        }
    ));
    
    mRegisteredEvents.emplace_back(EventDescription
    (
        {
            "backgrounds/events/3.bmp"
        },

        {
            "You discover a space-warping  wormhole that will allow faster movement of the vessel, but  with  most likely some structural damage in the process.",
            "You travel through the wormhole and significantly increase the vessel's maximum velocity, but also damaging it somewhat.",
            "You ignore the wormhole and continue with your mission."
        },

        {
            {
                EventOption("Use it. +.5 SPEED, -10 HP", 1, [&]()
                {
                    GameSingletons::SetPlayerMovementSpeedStat(GameSingletons::GetPlayerMovementSpeedStat() + 0.5f);
                    GameSingletons::SetPlayerCurrentHealth(math::Max(0.0f, GameSingletons::GetPlayerCurrentHealth() - 10.0f));
                    objectiveC_utils::Vibrate();
                    
                    if (GameSingletons::GetPlayerCurrentHealth() <= 0.0f)
                    {
                        mTransitioning = true;
                        mScene.SetProgressResetFlag();
                        mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAIN_MENU, "", true));
                    }
                }),
                EventOption("Ignore", 2, [](){}),
            },
            
            {
                EventOption("Leave", END_STATE_INDEX, [](){})
            },
            
            {
                EventOption("Leave", END_STATE_INDEX, [](){})
            }
        }
    ));

    mRegisteredEvents.emplace_back(EventDescription
    (
        {
            "backgrounds/events/1.bmp"
        },
     
        {
            "You discover a foreign planet filled  with abundant power crystal reserves that can be used to power the vessel's stats, and future research projects.",
            "You collected a few crystals and swiftly departed.",
            "You swiftly departed from the planet, ignoring the countless crystals around you."
        },
     
        {
            {
                EventOption("Collect 5 crytals", 1, [&](){ mUpgradeUnlockedHandler.OnUpgradeGained(game_constants::CRYSTALS_SMALL_EVENT_UPGRADE_NAME); }),
                EventOption("Ignore.", 2, [](){})
            },
            
            {
                EventOption("Leave.", END_STATE_INDEX, [](){})
            },
            
            {
                EventOption("Leave.", END_STATE_INDEX, [](){})
            }
        }
    ));
}

///------------------------------------------------------------------------------------------------

void EventUpdater::SelectRandomEvent()
{
    //mSelectedEvent = &mRegisteredEvents[0];
    mSelectedEvent = &mRegisteredEvents[static_cast<size_t>(math::ControlledRandomInt(0, static_cast<int>(mRegisteredEvents.size() - 1)))];
}

///------------------------------------------------------------------------------------------------

void EventUpdater::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();

    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = game_constants::MAP_BACKGROUND_SCALE;
        bgSO.mPosition = BACKGROUND_POSITION;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::BACKGROUND_TEXTURE_FILE_PATH + std::to_string(GameSingletons::GetBackgroundIndex()) + ".bmp"), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
    // Overlay
    {
        SceneObject overlaySo;
        overlaySo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::FULL_SCREEN_OVERLAY_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        overlaySo.mSceneObjectType = SceneObjectType::GUIObject;
        overlaySo.mScale = FULL_SCREEN_OVERLAY_SCALE;
        overlaySo.mPosition = FULL_SCREEN_OVERLAY_POSITION;
        overlaySo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.8f;
        mScene.AddSceneObject(std::move(overlaySo));
    }
    
    CreateEventSceneObjectsForCurrentState();
}

///------------------------------------------------------------------------------------------------

void EventUpdater::CreateEventSceneObjectsForCurrentState()
{
    mFadeInOptions = false;
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Cleanup of previous state
    if (mPreviousEventProgressionStateIndex != mEventProgressionStateIndex)
    {
        mScene.RemoveAllSceneObjectsWithName(EVENT_BACKGROUND_NAME);
        for (int i = 0; i < mSelectedEvent->mEventOptions[mPreviousEventProgressionStateIndex].size(); ++i)
        {
            mScene.RemoveAllSceneObjectsWithName(strutils::StringId(EVENT_OPTION_NAME_PREFIX + std::to_string(i)));
        }
        
        mTextPromptController = nullptr;
    }

    // Event background
    {
        const auto eventBackgroundIndex = math::Min(mSelectedEvent->mEventBackgroundTextureNames.size() - 1, mEventProgressionStateIndex);
        SceneObject eventBgSo;
        eventBgSo.mScale = EVENT_BACKGROUND_SCALE;
        eventBgSo.mPosition = EVENT_BACKGROUND_POSITION;
        eventBgSo.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + mSelectedEvent->mEventBackgroundTextureNames[eventBackgroundIndex]), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        eventBgSo.mSceneObjectType = SceneObjectType::WorldGameObject;
        eventBgSo.mName = EVENT_BACKGROUND_NAME;
        eventBgSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(eventBgSo));
    }
    
    // Event options
    {
        const auto eventOptionsIndex = math::Min(mSelectedEvent->mEventOptions.size() - 1, mEventProgressionStateIndex);
        const auto& currentEventOptions = mSelectedEvent->mEventOptions[eventOptionsIndex];
        for (int i = 0; i < currentEventOptions.size(); ++i)
        {
            SceneObject eventOptionSo;
            eventOptionSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
            eventOptionSo.mFontName = game_constants::DEFAULT_FONT_NAME;
            eventOptionSo.mSceneObjectType = SceneObjectType::GUIObject;
            eventOptionSo.mName = strutils::StringId(EVENT_OPTION_NAME_PREFIX + std::to_string(i));
            eventOptionSo.mText = currentEventOptions[i].mOptionText;
            
            eventOptionSo.mScale = EVENT_OPTIONS_FONT_SCALE;
            
            glm::vec2 rectBotLeft, rectTopRight;
            scene_object_utils::GetSceneObjectBoundingRect(eventOptionSo, rectBotLeft, rectTopRight);
            
            eventOptionSo.mPosition = EVENT_OPTIONS_TEXT_INIT_POSITION;
            eventOptionSo.mPosition.y -= EVENT_OPTIONS_TEXT_Y_INCREMENT * i;
            eventOptionSo.mPosition.x -= (math::Abs(rectBotLeft.x - rectTopRight.x)/2.0f);

            eventOptionSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f - 0.5f * i;
            
            mScene.AddSceneObject(std::move(eventOptionSo));
        }
    }
    
    // Text prompt for first progression
    const auto eventDescriptionIndex = math::Min(mSelectedEvent->mEventDescriptionTexts.size() - 1, mEventProgressionStateIndex);
    mTextPromptController = std::make_unique<TextPromptController>(mScene, TEXT_PROMPT_POSITION, TEXT_PROMPT_SCALE, TextPromptController::CharsAnchorMode::TOP_ANCHORED, true, mSelectedEvent->mEventDescriptionTexts[eventDescriptionIndex], [&](){ mFadeInOptions = true; });
}

///------------------------------------------------------------------------------------------------

void EventUpdater::CreateCrystalsTowardTargetPosition(const long crystalCount, const glm::vec3& position)
{
    for (int i = 0; i < crystalCount; ++i)
    {
        mFlows.emplace_back([this, position]()
        {
            auto& resService = resources::ResourceLoadingService::GetInstance();
            SceneObject crystalSo;
            
            glm::vec3 firstControlPoint(game_constants::GUI_CRYSTAL_POSITION);
            glm::vec3 thirdControlPoint(position);
            glm::vec3 secondControlPoint((thirdControlPoint + firstControlPoint) * 0.5f + glm::vec3(math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), math::RandomFloat(-DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG, DROPPED_CRYSTAL_SECOND_CONTROL_POINT_NOISE_MAG), 0.0f));
            
            firstControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
            secondControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
            thirdControlPoint.z = game_constants::GUI_CRYSTAL_POSITION.z;
            
            float speedNoise = math::RandomFloat(-DROPPED_CRYSTAL_SPEED/5, DROPPED_CRYSTAL_SPEED/5);
            float speedMultiplier = DROPPED_CRYSTAL_DISTANCE_FACTOR/glm::distance(thirdControlPoint, game_constants::GUI_CRYSTAL_POSITION);
            
            const strutils::StringId droppedCrystalName = strutils::StringId(std::to_string(SDL_GetPerformanceCounter()));
            
            crystalSo.mAnimation = std::make_unique<BezierCurvePathAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), math::BezierCurve({firstControlPoint, secondControlPoint, thirdControlPoint}), (DROPPED_CRYSTAL_SPEED + speedNoise) * speedMultiplier, false);
            crystalSo.mAnimation->SetCompletionCallback([=](){ mScene.RemoveAllSceneObjectsWithName(droppedCrystalName); });
            
            crystalSo.mExtraCompoundingAnimations.push_back(std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CRYSTALS_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::SMALL_CRYSTAL_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Y, 0.0f, game_constants::GUI_CRYSTAL_ROTATION_SPEED, false));
            
            crystalSo.mSceneObjectType = SceneObjectType::GUIObject;
            crystalSo.mPosition = firstControlPoint;
            crystalSo.mScale = game_constants::GUI_CRYSTAL_SCALE;
            crystalSo.mName = droppedCrystalName;
            mScene.AddSceneObject(std::move(crystalSo));
        }, i * game_constants::DROPPED_CRYSTALS_CREATION_STAGGER_MILLIS, RepeatableFlow::RepeatPolicy::ONCE);
    }
}

///------------------------------------------------------------------------------------------------
