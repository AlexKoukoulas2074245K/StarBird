///------------------------------------------------------------------------------------------------
///  LabUpdater.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "LabUpdater.h"
#include "Scene.h"
#include "SceneObjectUtils.h"
#include "TextPromptController.h"
#include "states/DebugConsoleGameState.h"
#include "datarepos/FontRepository.h"
#include "../resloading/ResourceLoadingService.h"
#include "../utils/Logging.h"

#include <vector>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

static const std::vector<game_constants::LabOptionType> DEFAULT_LAB_OPTIONS =
{
    game_constants::LabOptionType::REPAIR,
    game_constants::LabOptionType::CRYSTAL_TRANSFER,
    game_constants::LabOptionType::RESEARCH,
};

static const std::unordered_map<game_constants::LabOptionType, std::string> LAB_OPTION_DESCRIPTIONS =
{
    { game_constants::LabOptionType::REPAIR, "REPAIR:\n Fully repairs the vessel to factory state standards." },
    { game_constants::LabOptionType::CRYSTAL_TRANSFER, "CRYSTAL TRANSFER:\n Transfers all collected crystals to be stored and used for future pioneering human research." },
    { game_constants::LabOptionType::RESEARCH, "RESEARCH:\n Expends collected crystal reserves to unlock powerful upgrades for the vessel." }
};

static const strutils::StringId CONFIRMATION_BUTTON_NAME = strutils::StringId("CONFIRMATION_BUTTON");
static const strutils::StringId CONFIRMATION_BUTTON_TEXT_NAME = strutils::StringId("CONFIRMATION_BUTTON_TEXT");

static const char* RIGHT_NAVIGATION_ARROW_TEXTURE_FILE_NAME = "right_navigation_arrow_mm.bmp";
static const char* LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME = "left_navigation_arrow_mm.bmp";
static const char* CONFIRMATION_BUTTON_TEXTURE_FILE_NAME = "confirmation_button_mm.bmp";
static const char* TEXT_PROMPT_TEXTURE_FILE_NAME = "text_prompt_mm.bmp";

static const glm::vec3 LAB_BACKGROUND_POS = glm::vec3(-1.8f, 0.0f, -1.0f);
static const glm::vec3 LAB_BACKGROUND_SCALE = glm::vec3(28.0f, 28.0f, 1.0f);

static const glm::vec3 LAB_NAVIGATION_ARROW_SCALE = glm::vec3(3.0f, 2.0f, 0.0f);
static const glm::vec3 LAB_NAVIGATION_ARROW_POSITION = glm::vec3(-4.0f, 10.0f, 0.0f);

static const glm::vec3 LAB_CONFIRMATION_BUTTON_POSITION = glm::vec3(0.0f, -6.0f, 0.0f);
static const glm::vec3 LAB_CONFIRMATION_BTUTON_SCALE = glm::vec3(5.13f, 5.13f, 0.0f);

static const glm::vec3 LAB_CONFIRMATION_BUTTON_TEXT_POSITION = glm::vec3(-1.6f, -6.3f, 0.5f);
static const glm::vec3 LAB_CONFIRMATION_BUTTON_TEXT_SCALE = glm::vec3(0.01f, 0.01f, 1.0f);

static const glm::vec3 TEXT_PROMPT_POSITION = glm::vec3(0.0f, 7.2f, 0.5f);
static const glm::vec3 TEXT_PROMPT_SCALE = glm::vec3(12.0f, 10.0f, 1.0f);

static const float LAB_ARROW_PULSING_SPEED = 0.01f;
static const float LAB_ARROW_PULSING_ENLARGEMENT_FACTOR = 1.0f/100.0f;
static const float LAB_CAROUSEL_OBJECT_X_MULTIPLIER = 4.2f;
static const float LAB_CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT = 3.5f;
static const float LAB_CAROUSEL_ROTATION_THRESHOLD = 0.5f;
static const float LAB_CAROUSEL_ROTATION_SPEED = 0.006f;
static const float LAB_CONFIRMATION_BUTTON_ROTATION_SPEED = 0.0002f;

///------------------------------------------------------------------------------------------------

LabUpdater::LabUpdater(Scene& scene)
    : mScene(scene)
    , mStateMachine(&scene, nullptr, nullptr, nullptr)
    , mCarouselState(CarouselState::STATIONARY)
    , mSelectedLabOption(game_constants::LabOptionType::REPAIR)
    , mCarouselRads(0.0f)
    , mCarouselTargetRads(0.0f)
    , mTransitioning(false)
{
#ifdef DEBUG
    mStateMachine.RegisterState<DebugConsoleGameState>();
#endif

    auto& worldCamera = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject)->get();
    worldCamera.SetPosition(glm::vec3(0.0f));
    
    CreateSceneObjects();
    OnCarouselStationary();
}

///------------------------------------------------------------------------------------------------

LabUpdater::~LabUpdater()
{
}

///------------------------------------------------------------------------------------------------

PostStateUpdateDirective LabUpdater::VUpdate(std::vector<SceneObject>& sceneObjects, const float dtMillis)
{
    if (mStateMachine.Update(dtMillis) == PostStateUpdateDirective::BLOCK_UPDATE || mTransitioning)
    {
        return PostStateUpdateDirective::BLOCK_UPDATE;
    }
    
    auto camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
    auto& worldCamera = camOpt->get();
    
    // Input flow
    auto& inputContext = GameSingletons::GetInputContext();
    static glm::vec3 originalFingerDownTouchPos(0.0f);
    static bool exhaustedMove = false;
    
    if (inputContext.mEventType == SDL_FINGERDOWN)
    {
        originalFingerDownTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        auto navigationArrowSoOpt = mScene.GetSceneObject(game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME);
        
        if (navigationArrowSoOpt && scene_object_utils::IsPointInsideSceneObject(navigationArrowSoOpt->get(), originalFingerDownTouchPos))
        {
            mTransitioning = true;
            mScene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::MAP, "", true));
            return PostStateUpdateDirective::BLOCK_UPDATE;
        }
        
        auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
        if (confirmationButtonSoOpt && scene_object_utils::IsPointInsideSceneObject(*confirmationButtonSoOpt, originalFingerDownTouchPos))
        {
            OnConfirmationButtonPress();
        }
    }
    else if (inputContext.mEventType == SDL_FINGERMOTION && !exhaustedMove)
    {
        auto currentTouchPos = math::ComputeTouchCoordsInWorldSpace(GameSingletons::GetWindowDimensions(), GameSingletons::GetInputContext().mTouchPos, worldCamera.GetViewMatrix(), worldCamera.GetProjMatrix());
        
        if (mCarouselState == CarouselState::STATIONARY && math::Abs(originalFingerDownTouchPos.x - currentTouchPos.x) > LAB_CAROUSEL_ROTATION_THRESHOLD)
        {
            if (currentTouchPos.x > originalFingerDownTouchPos.x)
            {
                mCarouselState = CarouselState::MOVING_LEFT;
                mCarouselTargetRads = mCarouselRads + (math::PI * 2.0f / (mLabOptions.size()));
                OnCarouselMovementStart();
            }
            else
            {
                mCarouselState = CarouselState::MOVING_RIGHT;
                mCarouselTargetRads = mCarouselRads - (math::PI * 2.0f / (mLabOptions.size()));
                OnCarouselMovementStart();
            }
            
            exhaustedMove = true;
        }
    }
    else if (inputContext.mEventType == SDL_FINGERUP)
    {
        exhaustedMove = false;
    }
    
    // Rotate lab options
    if (mCarouselState == CarouselState::MOVING_LEFT)
    {
        mCarouselRads += dtMillis * LAB_CAROUSEL_ROTATION_SPEED;
        if (mCarouselRads >= mCarouselTargetRads)
        {
            mCarouselRads = mCarouselTargetRads;
            mCarouselState = CarouselState::STATIONARY;
            OnCarouselStationary();
        }
    }
    else if (mCarouselState == CarouselState::MOVING_RIGHT)
    {
        mCarouselRads -= dtMillis * LAB_CAROUSEL_ROTATION_SPEED;
        if (mCarouselRads <= mCarouselTargetRads)
        {
            mCarouselRads = mCarouselTargetRads;
            mCarouselState = CarouselState::STATIONARY;
            OnCarouselStationary();
        }
    }
    
    // Give fake perspective to all Lab options
    for (int i = 0; i < static_cast<int>(mLabOptions.size()); ++i)
    {
        auto labOptionSoOpt = mScene.GetSceneObject(strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i)));
        
        if (labOptionSoOpt)
        {
            auto& labOptionSo = labOptionSoOpt->get();
            PositionCarouselObject(labOptionSo, i);
        }
    }
    
    // Fade in confirmation button
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
        if (confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
        {
            confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        }
    }
    
    // & text
    auto confirmationButtonTextSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
    if (confirmationButtonTextSoOpt)
    {
        auto& confirmationButtonTextSo = confirmationButtonTextSoOpt->get();
        confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
        if (confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
        {
            confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
        }
    }
    
    // & text area
    if (mTextPromptController)
    {
        mTextPromptController->Update(dtMillis);
    }
    
    // Animate all SOs
    for (auto& sceneObject: sceneObjects)
    {
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
    
    return PostStateUpdateDirective::CONTINUE;
}

///------------------------------------------------------------------------------------------------

void LabUpdater::VOnAppStateChange(Uint32 event)
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

std::string LabUpdater::VGetDescription() const
{
    return "";
}

///------------------------------------------------------------------------------------------------

#ifdef DEBUG
void LabUpdater::VOpenDebugConsole()
{
    if (mStateMachine.GetActiveStateName() != DebugConsoleGameState::STATE_NAME)
    {
        mStateMachine.PushState(DebugConsoleGameState::STATE_NAME);
    }
}
#endif

///------------------------------------------------------------------------------------------------

void LabUpdater::CreateSceneObjects()
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    // Background
    {
        SceneObject bgSO;
        bgSO.mScale = LAB_BACKGROUND_SCALE;
        bgSO.mPosition = LAB_BACKGROUND_POS;
        bgSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_BACKGROUND_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        bgSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        bgSO.mName = game_constants::BACKGROUND_SCENE_OBJECT_NAME;
        bgSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        mScene.AddSceneObject(std::move(bgSO));
    }
    
//    // Navigation arrow
//    {
//        SceneObject arrowSo;
//        arrowSo.mPosition = game_constants::LAB_NAVIGATION_ARROW_POSITION;
//        arrowSo.mScale = game_constants::LAB_NAVIGATION_ARROW_SCALE;
//        arrowSo.mAnimation = std::make_unique<PulsingAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LEFT_NAVIGATION_ARROW_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), 0.0f, game_constants::LAB_ARROW_PULSING_SPEED, game_constants::LAB_ARROW_PULSING_ENLARGEMENT_FACTOR, false);
//        arrowSo.mSceneObjectType = SceneObjectType::WorldGameObject;
//        arrowSo.mName = game_constants::NAVIGATION_ARROW_SCENE_OBJECT_NAME;
//        arrowSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
//        mScene.AddSceneObject(std::move(arrowSo));
//    }
//
    // Options
    const auto optionCount = DEFAULT_LAB_OPTIONS.size();
    mLabOptions.resize(optionCount);
    
    for (int i = 0; i < static_cast<int>(mLabOptions.size()); ++i)
    {
        game_constants::LabOptionType currentLabOption = static_cast<game_constants::LabOptionType>(i);
        mLabOptions[i] = currentLabOption;
        
        SceneObject labOptionSO;
        labOptionSO.mAnimation = std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::LAB_OPTION_TYPE_TO_TEXTURE.at(currentLabOption)), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME), glm::vec3(1.0f), false);
        labOptionSO.mSceneObjectType = SceneObjectType::WorldGameObject;
        labOptionSO.mName = strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i));
        labOptionSO.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        PositionCarouselObject(labOptionSO, i);
        mScene.AddSceneObject(std::move(labOptionSO));
    }
}

///------------------------------------------------------------------------------------------------

void LabUpdater::PositionCarouselObject(SceneObject& carouselObject, const int objectIndex) const
{
    float optionRadsOffset = objectIndex * (math::PI * 2.0f / mLabOptions.size());
    carouselObject.mPosition.x = math::Sinf(mCarouselRads + optionRadsOffset) * LAB_CAROUSEL_OBJECT_X_MULTIPLIER;
    carouselObject.mPosition.z = game_constants::LAB_OPTIONS_Z + math::Cosf(mCarouselRads + optionRadsOffset);
    carouselObject.mScale = glm::vec3(carouselObject.mPosition.z + LAB_CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT, carouselObject.mPosition.z + LAB_CAROUSEL_OBJECT_SCALE_CONSTANT_INCREMENT, 1.0f);
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnCarouselMovementStart()
{
    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_NAME);
    mScene.RemoveAllSceneObjectsWithName(CONFIRMATION_BUTTON_TEXT_NAME);
    mTextPromptController.reset();
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnCarouselStationary()
{
    // Find front-most option
    auto closestZ = -1.0f;
    for (int i = 0; i < static_cast<int>(mLabOptions.size()); ++i)
    {
        auto labOptionSoOpt = mScene.GetSceneObject(strutils::StringId(game_constants::LAB_OPTION_NAME_PREFIX.GetString() + std::to_string(i)));
        
        if (labOptionSoOpt && labOptionSoOpt->get().mPosition.z > closestZ)
        {
            closestZ = labOptionSoOpt->get().mPosition.z;
            mSelectedLabOption = static_cast<game_constants::LabOptionType>(i);
        }
    }
    
    // Recreate confirmation button
    auto& resService = resources::ResourceLoadingService::GetInstance();
    SceneObject confirmationButtonSo;
    confirmationButtonSo.mPosition = LAB_CONFIRMATION_BUTTON_POSITION;
    confirmationButtonSo.mScale = LAB_CONFIRMATION_BTUTON_SCALE;
    confirmationButtonSo.mAnimation = std::make_unique<RotationAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CONFIRMATION_BUTTON_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), RotationAnimation::RotationMode::ROTATE_CONTINUALLY, RotationAnimation::RotationAxis::Z, 0.0f, LAB_CONFIRMATION_BUTTON_ROTATION_SPEED, false);
    confirmationButtonSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    confirmationButtonSo.mName = CONFIRMATION_BUTTON_NAME;
    confirmationButtonSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    confirmationButtonSo.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
    mScene.AddSceneObject(std::move(confirmationButtonSo));
    
    // Confirmation button text
    SceneObject confirmationButtonTextSo;
    confirmationButtonTextSo.mPosition = LAB_CONFIRMATION_BUTTON_TEXT_POSITION;
    confirmationButtonTextSo.mScale = LAB_CONFIRMATION_BUTTON_TEXT_SCALE;
    confirmationButtonTextSo.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
    confirmationButtonTextSo.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
    confirmationButtonTextSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    confirmationButtonTextSo.mName = CONFIRMATION_BUTTON_TEXT_NAME;
    confirmationButtonTextSo.mText = "Confirm";
    confirmationButtonTextSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mScene.AddSceneObject(std::move(confirmationButtonTextSo));
    
    // Text Prompt
    mTextPromptController = std::make_unique<TextPromptController>(mScene, std::make_unique<SingleFrameAnimation>(resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + TEXT_PROMPT_TEXTURE_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false), TEXT_PROMPT_POSITION, TEXT_PROMPT_SCALE, !mVisitedLabOptions.contains(mSelectedLabOption), LAB_OPTION_DESCRIPTIONS.at(mSelectedLabOption));
    
    
    mVisitedLabOptions.insert(mSelectedLabOption);
}

///------------------------------------------------------------------------------------------------

void LabUpdater::OnConfirmationButtonPress()
{
    switch (mSelectedLabOption)
    {
        case game_constants::LabOptionType::REPAIR: break;
        case game_constants::LabOptionType::CRYSTAL_TRANSFER: break;
        case game_constants::LabOptionType::RESEARCH: break;
    }
    
    auto confirmationButtonSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_NAME);
    auto confirmationButtonTexSoOpt = mScene.GetSceneObject(CONFIRMATION_BUTTON_TEXT_NAME);
    
    if (confirmationButtonSoOpt)
    {
        auto& confirmationButtonSo = confirmationButtonSoOpt->get();
        confirmationButtonSo.mScale = LAB_CONFIRMATION_BTUTON_SCALE;
        
        confirmationButtonSo.mExtraCompoundingAnimations.clear();
        confirmationButtonSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonSo.mAnimation->VGetCurrentShaderResourceId(), LAB_CONFIRMATION_BTUTON_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, LAB_ARROW_PULSING_SPEED * 2, LAB_ARROW_PULSING_ENLARGEMENT_FACTOR * 10, false));
    }
    
    if (confirmationButtonTexSoOpt)
    {
        auto& confirmationButtonTextSo = confirmationButtonTexSoOpt->get();
        confirmationButtonTextSo.mScale = LAB_CONFIRMATION_BUTTON_TEXT_SCALE;
        
        confirmationButtonTextSo.mExtraCompoundingAnimations.clear();
        confirmationButtonTextSo.mExtraCompoundingAnimations.push_back(std::make_unique<PulsingAnimation>(confirmationButtonTextSo.mAnimation->VGetCurrentTextureResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentMeshResourceId(), confirmationButtonTextSo.mAnimation->VGetCurrentShaderResourceId(), LAB_CONFIRMATION_BUTTON_TEXT_SCALE, PulsingAnimation::PulsingMode::INNER_PULSE_ONCE, 0.0f, LAB_ARROW_PULSING_SPEED * 2, LAB_ARROW_PULSING_ENLARGEMENT_FACTOR / 40, false));
    }
    
    GameSingletons::SetPlayerCurrentHealth(100);
}

///------------------------------------------------------------------------------------------------
