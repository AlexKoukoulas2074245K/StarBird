///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Game.h"
#include "GameSingletons.h"
#include "InputContext.h"
#include "PersistenceUtils.h"
#include "Scene.h"

#include "dataloaders/UpgradesLoader.h"

#include "datarepos/FontRepository.h"
#include "datarepos/ObjectTypeDefinitionRepository.h"
#include "datarepos/WaveBlocksRepository.h"

#include "../resloading/ResourceLoadingService.h"

#include "../utils/Logging.h"
#include "../utils/MathUtils.h"
#include "../utils/ObjectiveCUtils.h"
#include "../utils/OpenGL.h"
#include "../utils/OSMessageBox.h"

#include <SDL.h>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

Game::Game()
    : mIsFinished(false)
{
    if (!InitSystems()) return;
    Run();
}

///------------------------------------------------------------------------------------------------

Game::~Game()
{
    SDL_Quit();
}

///------------------------------------------------------------------------------------------------

bool Game::InitSystems()
{
    // Initialize SDL
    if(SDL_Init( SDL_INIT_VIDEO ) < 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    // Set texture filtering to linear
    if(!SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1"))
    {
        Log(LogType::WARNING, "Warning: Linear texture filtering not enabled!");
    }

    // Get device display mode
    SDL_DisplayMode displayMode;
    
    int windowWidth, windowHeight;
    if(SDL_GetCurrentDisplayMode(0, &displayMode) != 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    // .. and window dimensions
    windowWidth = displayMode.w;
    windowHeight = displayMode.h;
    
    // Create window
    auto* window = SDL_CreateWindow("StarBird", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS);
    
    if(!window)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    GameSingletons::SetWindow(window);
    GameSingletons::SetWindowDimensions(windowWidth, windowHeight);
    
    // Set OpenGL desired attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    // Create OpenGL context
    auto* context = SDL_GL_CreateContext(window);
    if (SDL_GL_MakeCurrent(window, context) != 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    // Enable texture blending
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    // Enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glDepthFunc(GL_LESS));
    
    Log(LogType::INFO, "Vendor     : %s", GL_NO_CHECK_CALL(glGetString(GL_VENDOR)));
    Log(LogType::INFO, "Renderer   : %s", GL_NO_CHECK_CALL(glGetString(GL_RENDERER)));
    Log(LogType::INFO, "Version    : %s", GL_NO_CHECK_CALL(glGetString(GL_VERSION)));
    
    return true;
}

///------------------------------------------------------------------------------------------------

void Game::Run()
{
    InitPersistentData();
    
    Scene scene;
    scene.ChangeScene(Scene::TransitionParameters(Scene::SceneType::CHEST_REWARD, "test_level_with_boss", false));
    
    SDL_Event e;
    
    auto lastFrameMillisSinceInit         = 0.0f;
    auto secsAccumulator                  = 0.0f;
    auto framesAccumulator                = 0LL;
    
    GameSingletons::SetInputContextEvent(SDL_FINGERUP);
    
    std::unordered_map<SDL_FingerID, glm::vec2> multiTouchMotionframeFingerIDsToTouchPositions;
    
    //While application is running
    while(!mIsFinished)
    {
        // Calculate frame delta
        const auto currentMillisSinceInit = static_cast<float>(SDL_GetTicks());        // the number of milliseconds since the SDL library
        const auto dtMillis = currentMillisSinceInit - lastFrameMillisSinceInit; // millis diff between current and last frame
        
        lastFrameMillisSinceInit = currentMillisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMillis * 0.001f; // dt in seconds;
        
        //Handle events on queue
        auto lastAppForegroundBackgroundEvent = 0;
        
        multiTouchMotionframeFingerIDsToTouchPositions.clear();
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            switch (e.type)
            {
                case SDL_QUIT:
                case SDL_APP_TERMINATING:
                {
                    mIsFinished = true;
                } break;
                case SDL_FINGERDOWN:
                case SDL_FINGERUP:
                case SDL_FINGERMOTION:
                {
                    GameSingletons::SetInputContextEvent(e.type);
                    GameSingletons::SetInputContextTouchPos(glm::vec2(e.tfinger.x, e.tfinger.y));
                    
                    if (e.type == SDL_FINGERUP)
                    {
                        GameSingletons::SetInputContextMultiGestureActive(false);
                    }
                    else
                    {
                        multiTouchMotionframeFingerIDsToTouchPositions[e.tfinger.fingerId] = glm::vec2(e.tfinger.x, e.tfinger.y);
                    }
                } break;
                
                case SDL_KEYDOWN:
                {
                    const auto keyCode = e.key.keysym.scancode;
                    
                    GameSingletons::SetInputContextEvent(e.type);
                    GameSingletons::SetInputContextKey(keyCode);
                    
                    if (keyCode == SDL_SCANCODE_BACKSPACE)
                    {
                        const auto& currentText = GameSingletons::GetInputContext().mText;
                        if (!currentText.empty())
                        {
                            GameSingletons::SetInputContextText(currentText.substr(0, currentText.size() - 1));
                        }
                    }
                    else if (keyCode != SDL_SCANCODE_RETURN && keyCode != SDL_SCANCODE_UP && keyCode != SDL_SCANCODE_DOWN && !SDL_IsScreenKeyboardShown(GameSingletons::GetWindow()))
                    {
                        if (keyCode == SDL_SCANCODE_GRAVE)
                        {
#ifdef DEBUG
                            scene.OpenDebugConsole();
#endif
                        }
                        else if ((e.key.keysym.mod & KMOD_LSHIFT) == 0 && (e.key.keysym.mod & KMOD_RSHIFT) == 0)
                        {
                            OnTextInput(std::string(1, e.key.keysym.sym));
                        }
                        else
                        {
                            if (e.key.keysym.sym >= 'a' && e.key.keysym.sym <= 'z')
                            {
                                OnTextInput(std::string(1, e.key.keysym.sym - 32));
                            }
                            else
                            {
                                switch (e.key.keysym.sym)
                                {
                                    case SDLK_1:            OnTextInput("!"); break;
                                    case SDLK_2:            OnTextInput("@"); break;
                                    case SDLK_3:            OnTextInput("£"); break;
                                    case SDLK_4:            OnTextInput("$"); break;
                                    case SDLK_5:            OnTextInput("%"); break;
                                    case SDLK_6:            OnTextInput("^"); break;
                                    case SDLK_7:            OnTextInput("&"); break;
                                    case SDLK_8:            OnTextInput("*"); break;
                                    case SDLK_9:            OnTextInput("("); break;
                                    case SDLK_0:            OnTextInput(")"); break;
                                    case SDLK_MINUS:        OnTextInput("_"); break;
                                    case SDLK_EQUALS:       OnTextInput("+"); break;
                                    case SDLK_LEFTBRACKET:  OnTextInput("{"); break;
                                    case SDLK_RIGHTBRACKET: OnTextInput("}"); break;
                                    case SDLK_SEMICOLON:    OnTextInput(":"); break;
                                    case SDLK_QUOTE:        OnTextInput("\""); break;
                                    case SDLK_LESS:         OnTextInput("<"); break;
                                    case SDLK_GREATER:      OnTextInput(">"); break;
                                
                                    default: Log(LogType::WARNING, "Unhandled input %s with pressed shift", std::string(1, e.key.keysym.sym).c_str());
                                }
                            }
                        }
                    }
                    
                } break;
                case SDL_KEYUP:
                {
                    GameSingletons::SetInputContextEvent(e.type);
                } break;
                    
                case SDL_TEXTINPUT:
                {
                    OnTextInput(e.text.text);
                } break;
                
                case SDL_APP_WILLENTERBACKGROUND:
                case SDL_APP_DIDENTERBACKGROUND:
                case SDL_APP_WILLENTERFOREGROUND:
                case SDL_APP_DIDENTERFOREGROUND:
                {
                    lastAppForegroundBackgroundEvent = e.type;
                } break;
            }
        }
//        
        auto maxFoundPinchDistance = 0.0f;
        if (multiTouchMotionframeFingerIDsToTouchPositions.size() > 1)
        {
            for (auto outerEntry: multiTouchMotionframeFingerIDsToTouchPositions)
            {
                for (auto innerEntry: multiTouchMotionframeFingerIDsToTouchPositions)
                {
                    if (outerEntry.first == innerEntry.first) continue;
                    
                    auto pointDistance = glm::distance(outerEntry.second, innerEntry.second);
                    if (maxFoundPinchDistance < pointDistance)
                    {
                        maxFoundPinchDistance = pointDistance;
                        GameSingletons::SetInputContextMultiGestureActive(true);
                    }
                }
            }
        }
        
        GameSingletons::SetInputContextPinchDistance(maxFoundPinchDistance);
        
        if (secsAccumulator > 1.0f)
        {
            Log(LogType::INFO, "FPS: %d | %s", framesAccumulator, scene.GetSceneStateDescription().c_str());
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
        }
        
        scene.UpdateScene(math::Min(20.0f, dtMillis));
        scene.RenderScene();

        if (lastAppForegroundBackgroundEvent)
        {
            scene.OnAppStateChange(lastAppForegroundBackgroundEvent);
        }
    }
}

///------------------------------------------------------------------------------------------------

void Game::InitPersistentData()
{
    auto& waveBlocksRepo = WaveBlocksRepository::GetInstance();
    waveBlocksRepo.LoadWaveBlocks();
    
    UpgradesLoader loader;
    GameSingletons::SetAvailableUpgrades(loader.LoadAllUpgrades());
    
    auto& typeDefRepo = ObjectTypeDefinitionRepository::GetInstance();
    typeDefRepo.LoadObjectTypeDefinition(game_constants::PLAYER_OBJECT_TYPE_DEF_NAME);
    
    if (!persistence_utils::ProgressSaveFileExists())
    {
        persistence_utils::GenerateNewProgressSaveFile();
    }
    else
    {
        persistence_utils::LoadFromProgressSaveFile();
    }
}

///------------------------------------------------------------------------------------------------

void Game::OnTextInput(const std::string& text)
{
    GameSingletons::SetInputContextEvent(SDL_TEXTINPUT);
    GameSingletons::SetInputContextText(GameSingletons::GetInputContext().mText + text);
}

///------------------------------------------------------------------------------------------------
