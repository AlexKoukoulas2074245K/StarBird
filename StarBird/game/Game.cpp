///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "Game.h"
#include "InputContext.h"
#include "PhysicsConstants.h"
#include "RepeatableFlow.h"
#include "Scene.h"
#include "SceneObjectConstants.h"

#include "../utils/Logging.h"
#include "../utils/MathUtils.h"
#include "../utils/OpenGL.h"
#include "../utils/OSMessageBox.h"
#include "../resloading/ResourceLoadingService.h"
#include "../resloading/MeshResource.h"
#include "../resloading/ShaderResource.h"
#include "../resloading/TextureResource.h"

#include <Box2D/Box2D.h>
#include <SDL.h>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

Game::Game()
    : mIsFinished(false)
{
    if (!InitSystems()) return;
    if (!LoadAssets()) return;
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

resources::ResourceId gEnemyTexture;
resources::ResourceId gPlayerTexture;
resources::ResourceId gJoystickTexture;
resources::ResourceId gJoystickBoundsTexture;
resources::ResourceId gBulletTexture;
resources::ResourceId gInvisWallTexture;
resources::ResourceId gSpaceBackgroundTexture;

resources::ResourceId gBasicShader;
resources::ResourceId gTexOffsetShader;

resources::ResourceId gMesh;
bool Game::LoadAssets()
{
    gEnemyTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "enemy.bmp");
    
    gPlayerTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "player.bmp");
    
    gJoystickTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "joystick.bmp");
    
    gJoystickBoundsTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "joystick_bounds.bmp");
    
    gBulletTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "bullet.bmp");
    
    gInvisWallTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "debug.bmp");
    
    gSpaceBackgroundTexture = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "space_bg.bmp");
    
    gBasicShader = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    
    gTexOffsetShader = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "texoffset.vs");
    
    gMesh = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + "quad.obj");
    
    return true;
}

///------------------------------------------------------------------------------------------------

void createPlayer(b2World* world, SceneObject& so, float x = 0.0f, float y = -10.0f)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(x, y);
    b2Body* body = world->CreateBody(&bodyDef);
    
    b2PolygonShape dynamicBox;
    
    auto& playerTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gPlayerTexture);
    
    float textureAspect = playerTexture.GetDimensions().x/playerTexture.GetDimensions().y;
    dynamicBox.SetAsBox(1.0f, 1.0f/textureAspect);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.0f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PLAYER_CATEGORY_BIT;
    fixtureDef.filter.maskBits &= (~(PLAYER_CATEGORY_BIT) | ENEMY_CATEGORY_BIT);
    body->CreateFixture(&fixtureDef);
    
    so.mBody = body;
    so.mShaderResourceId = gBasicShader;
    so.mTextureResourceId = gPlayerTexture;
    so.mMeshResourceId = gMesh;
    so.mSceneObjectType = SceneObjectType::GameObject;
    so.mCustomPosition.z = 0.0f;
    so.mNameTag = sceneobject_constants::PLAYER_SCENE_OBJECT_NAME;
    so.mObjectFamilyTypeName = strutils::StringId("player");
}

void createBullet(b2World* world, b2Body* player, SceneObject& so)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = player->GetWorldCenter();
    bodyDef.bullet =  true;
    b2Body* body = world->CreateBody(&bodyDef);
    body->SetLinearVelocity(b2Vec2(0.0f, 8.0f));
    b2PolygonShape dynamicBox;
    
    auto& bulletTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gBulletTexture);
    
    float textureAspect = bulletTexture.GetDimensions().x/bulletTexture.GetDimensions().y;
    dynamicBox.SetAsBox(0.25f, 0.25f/textureAspect);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 0.1f;
    fixtureDef.friction = 0.0f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PLAYER_BULLET_CATEGORY_BIT;
    fixtureDef.filter.maskBits &= (~PLAYER_CATEGORY_BIT);
    fixtureDef.filter.maskBits &= (~PLAYER_BULLET_CATEGORY_BIT);
    
    body->CreateFixture(&fixtureDef);
    
    so.mBody = body;
    so.mCustomPosition.z = -0.5f;
    so.mShaderResourceId = gBasicShader;
    so.mTextureResourceId = gBulletTexture;
    so.mMeshResourceId = gMesh;
    so.mSceneObjectType = SceneObjectType::GameObject;
    so.mNameTag.fromAddress(so.mBody);
}

#define L_WALL
#define R_WALL
#define B_WALL
void Game::Run()
{
    b2World world(b2Vec2(0.0f, 0.0f));
    Scene scene(world);
    scene.LoadLevel("test_level");
    
    class ContactListener : public b2ContactListener
    {
    public:
        ContactListener(Scene& scene): mScene(scene) {}
        
        void BeginContact(b2Contact* contact) override
        {
            const auto catA = contact->GetFixtureA()->GetFilterData().categoryBits;
            const auto catB = contact->GetFixtureB()->GetFilterData().categoryBits;
            
            if (catA == ENEMY_CATEGORY_BIT && catB == PLAYER_BULLET_CATEGORY_BIT)
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
            else if (catA == PLAYER_BULLET_CATEGORY_BIT && catB == ENEMY_CATEGORY_BIT)
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
    
    ContactListener cl(scene);
    world.SetContactListener(&cl);
    
    
    auto* window = SDL_GL_GetCurrentWindow();
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    
    Camera cam(windowWidth, windowHeight, 30.0f);
    Camera guiCam(windowWidth, windowHeight, 30.0f);
    
#ifdef GROUND
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);

    b2Body* groundBody = world.CreateBody(&groundBodyDef);

    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 15.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);
#endif
    
    {
        SceneObject bgSO;
        bgSO.mCustomScale = glm::vec3(28.0f, 28.0f, 1.0f);
        bgSO.mCustomPosition.z = -1.0f;
        bgSO.mMeshResourceId = gMesh;
        bgSO.mShaderResourceId = gTexOffsetShader;
        bgSO.mTextureResourceId = gSpaceBackgroundTexture;
        bgSO.mSceneObjectType = SceneObjectType::GUIObject;
        bgSO.mNameTag = sceneobject_constants::BACKGROUND_SCENE_OBJECT_NAME;
        scene.AddSceneObject(std::move(bgSO));
    }
    
    {
        SceneObject playerSO;
        createPlayer(&world, playerSO);
        scene.AddSceneObject(std::move(playerSO));
    }
    
#ifdef L_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(-cam.GetCameraLenseWidth()/2.0f, 0.0f);
        
        b2Body* wallBody = world.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, cam.GetCameraLenseHeight()/2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 10.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = LEVEL_BOUNDS_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = gBasicShader;
        so.mTextureResourceId = gInvisWallTexture;
        so.mMeshResourceId = gMesh;
        so.mSceneObjectType = SceneObjectType::GameObject;
        scene.AddSceneObject(std::move(so));
    }
#endif
    
#ifdef R_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(cam.GetCameraLenseWidth()/2, 0.0f);
        
        b2Body* wallBody = world.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(1.0f, cam.GetCameraLenseHeight()/2.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 10.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = LEVEL_BOUNDS_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = gBasicShader;
        so.mTextureResourceId = gInvisWallTexture;
        so.mMeshResourceId = gMesh;
        so.mSceneObjectType = SceneObjectType::GameObject;
        scene.AddSceneObject(std::move(so));
    }
#endif
    
#ifdef B_WALL
    {
        b2BodyDef wallBodyDef;
        wallBodyDef.position.Set(0.0f, -cam.GetCameraLenseHeight()/2 + 1.0f);
        
        b2Body* wallBody = world.CreateBody(&wallBodyDef);

        b2PolygonShape wallShape;
        wallShape.SetAsBox(cam.GetCameraLenseWidth()/2.0, 1.0f);
        
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &wallShape;
        fixtureDef.density = 10.0f;
        fixtureDef.friction = 0.0f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = LEVEL_BOUNDS_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        SceneObject so;
        so.mBody = wallBody;
        so.mShaderResourceId = gBasicShader;
        so.mTextureResourceId = gInvisWallTexture;
        so.mMeshResourceId = gMesh;
        so.mSceneObjectType = SceneObjectType::GameObject;
        scene.AddSceneObject(std::move(so));
    }
    
    {
        SceneObject joystickSO;
        joystickSO.mShaderResourceId = gBasicShader;
        joystickSO.mTextureResourceId = gJoystickTexture;
        joystickSO.mMeshResourceId = gMesh;
        joystickSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickSO.mCustomScale = glm::vec3(2.0f, 2.0f, 1.0f);
        joystickSO.mNameTag = sceneobject_constants::JOYSTICK_SCENE_OBJECT_NAME;
        joystickSO.mInvisible = true;
        scene.AddSceneObject(std::move(joystickSO));
    }
    
    {
        SceneObject joystickBoundsSO;
        joystickBoundsSO.mShaderResourceId = gBasicShader;
        joystickBoundsSO.mTextureResourceId = gJoystickBoundsTexture;
        joystickBoundsSO.mMeshResourceId = gMesh;
        joystickBoundsSO.mSceneObjectType = SceneObjectType::GUIObject;
        joystickBoundsSO.mCustomScale = glm::vec3(4.0f, 4.0f, 1.0f);
        joystickBoundsSO.mNameTag = sceneobject_constants::JOYSTICK_BOUNDS_SCENE_OBJECT_NAME;
        joystickBoundsSO.mInvisible = true;
        scene.AddSceneObject(std::move(joystickBoundsSO));
    }
#endif
    
    std::vector<RepeatableFlow> flows;
    //int spawnCount = 0;
    flows.emplace_back([&]()
    {
        SceneObject so;
        createBullet(&world, scene.GetSceneObject(sceneobject_constants::PLAYER_SCENE_OBJECT_NAME)->get().mBody, so);
        scene.AddSceneObject(std::move(so));
    }, 300.0f, RepeatableFlow::RepeatPolicy::REPEAT);
    
    int velocityIterations = 6;
    int positionIterations = 2;
    float timeStep = 1.0f / 60.0f;
    SDL_Event e;
    
    auto lastFrameMilisSinceInit = 0.0f;
    auto secsAccumulator         = 0.0f;
    auto framesAccumulator       = 0LL;
    
    std::unique_ptr<b2Vec2> attractionPoint = nullptr;
    InputContext inputContext = {};
    inputContext.mLastEventType = SDL_FINGERUP;
    
    //While application is running
    while(!mIsFinished)
    {
        // Calculate frame delta
        const auto currentMilisSinceInit = static_cast<float>(SDL_GetTicks());        // the number of milliseconds since the SDL library
        const auto dtMilis = currentMilisSinceInit - lastFrameMilisSinceInit; // milis diff between current and last frame
        
        lastFrameMilisSinceInit = currentMilisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMilis * 0.001f; // dt in seconds;
        
        
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            switch (e.type)
            {
                case SDL_QUIT:
                {
                    mIsFinished = true;
                } break;
                case SDL_FINGERDOWN:
                case SDL_FINGERUP:
                case SDL_FINGERMOTION:
                {
                    inputContext.mLastEventType = e.type;
                    inputContext.mTouchPos = glm::vec2(e.tfinger.x, e.tfinger.y);
                } break;
                default: break;
            }
        }
        
        if (secsAccumulator > 1.0f)
        {
            Log(LogType::INFO, "FPS: %d Body count %d", framesAccumulator, world.GetBodyCount());
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
        }

        for (auto& flow: flows) flow.update(dtMilis);
        
        world.Step(timeStep, velocityIterations, positionIterations);
        
        scene.UpdateScene(dtMilis, inputContext);
        scene.RenderScene();
    }
}

///------------------------------------------------------------------------------------------------
