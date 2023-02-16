///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Game.h"
#include "GameSingletons.h"
#include "InputContext.h"
#include "Scene.h"

#include "../resloading/ResourceLoadingService.h"

#include "../utils/Logging.h"
#include "../utils/MathUtils.h"
#include "../utils/OpenGL.h"
#include "../utils/OSMessageBox.h"

#include <SDL.h>

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
    auto* window = SDL_CreateWindow("StarBird", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    
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
    
    // Set fallback texture
    resources::ResourceLoadingService::GetInstance().SetFallbackTexture(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "debug.bmp");
    
    Log(LogType::INFO, "Vendor     : %s", GL_NO_CHECK_CALL(glGetString(GL_VENDOR)));
    Log(LogType::INFO, "Renderer   : %s", GL_NO_CHECK_CALL(glGetString(GL_RENDERER)));
    Log(LogType::INFO, "Version    : %s", GL_NO_CHECK_CALL(glGetString(GL_VERSION)));
    
    return true;
}

///------------------------------------------------------------------------------------------------

void Game::Run()
{
    Scene scene;
    scene.LoadLevel("test_level");
    
    SDL_Event e;
    
    auto lastFrameMillisSinceInit = 0.0f;
    auto secsAccumulator         = 0.0f;
    auto framesAccumulator       = 0LL;
    
    GameSingletons::SetInputContextEvent(SDL_FINGERUP);
    
    //While application is running
    while(!mIsFinished)
    {
        // Calculate frame delta
        const auto currentMillisSinceInit = static_cast<float>(SDL_GetTicks());        // the number of milliseconds since the SDL library
        const auto dtMillis = currentMillisSinceInit - lastFrameMillisSinceInit; // milis diff between current and last frame
        
        lastFrameMillisSinceInit = currentMillisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMillis * 0.001f; // dt in seconds;
        
        
        //Handle events on queue
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
                } break;
                default: break;
                    
                case SDL_APP_WILLENTERBACKGROUND:
                case SDL_APP_DIDENTERBACKGROUND:
                case SDL_APP_WILLENTERFOREGROUND:
                case SDL_APP_DIDENTERFOREGROUND:
                {
                    scene.OnAppStateChange(e.type);
                } break;
            }
        }
        
        if (secsAccumulator > 1.0f)
        {
            Log(LogType::INFO, "FPS: %d | %s", framesAccumulator, scene.GetSceneStateDescription().c_str());
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
        }
        
        scene.UpdateScene(math::Min(20.0f, dtMillis));
        scene.RenderScene();
    }
}

///------------------------------------------------------------------------------------------------
