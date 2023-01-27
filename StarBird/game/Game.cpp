///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "Game.h"

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

    Log(LogType::INFO, "Vendor     : %s\n", GL_NO_CHECK_CALL(glGetString(GL_VENDOR)));
    Log(LogType::INFO, "Renderer   : %s\n", GL_NO_CHECK_CALL(glGetString(GL_RENDERER)));
    Log(LogType::INFO, "Version    : %s\n", GL_NO_CHECK_CALL(glGetString(GL_VERSION)));
    
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
    
    gMesh = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + "cube.obj");
    
    return true;
}

///------------------------------------------------------------------------------------------------

using namespace strutils;

std::unordered_map<b2Body*, int> health;

class DelayedFlow
{
public:
    enum class RepeatPolicy
    {
        ONCE, REPEAT
    };
    
    using CallbackT = std::function<void()>;
    
    DelayedFlow(CallbackT callback, float afterMs, RepeatPolicy repeatPolicy):
          callback_(callback)
        , targetDuration_(afterMs)
        , ticksLeft_(afterMs)
        , repeatPolicy_(repeatPolicy)
        , isRunning_(true)
    {}
    
    void update(float dt)
    {
        if (!isRunning_) return;
        
        ticksLeft_ -= dt;
        if (ticksLeft_ <= 0.0f)
        {
            callback_();
            if (repeatPolicy_ == RepeatPolicy::REPEAT)
            {
                ticksLeft_ = targetDuration_;
            }
            else
            {
                isRunning_ = false;
            }
        }
    }
    
private:
    CallbackT callback_;
    float targetDuration_;
    float ticksLeft_;
    RepeatPolicy repeatPolicy_;
    bool isRunning_;
};

void renderSpaceBackground(float offset, Camera& c)
{
    auto& meshResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh);
    
    auto& shaderResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::ShaderResource>(gTexOffsetShader);
    
    GL_CALL(glUseProgram(shaderResource.GetProgramId()));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gSpaceBackgroundTexture).GetGLTextureId()));
    
    glm::mat4 world(1.0f);
    world = glm::scale(world, glm::vec3(28.0f, 28.0f, 1.0f));
    
    shaderResource.SetFloat(StringId("texoffset"), offset);
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
}

void renderJoystick(b2Vec2 b2pos, b2Vec2 b2InitPos, Camera& c)
{
    auto& meshResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh);
    
    auto& shaderResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::ShaderResource>(gBasicShader);
    
    GL_CALL(glUseProgram(shaderResource.GetProgramId()));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gJoystickTexture).GetGLTextureId()));
    
    glm::mat4 world(1.0f);
    world = glm::translate(world, glm::vec3(b2pos.x, b2pos.y, 0.0f));
    world = glm::scale(world, glm::vec3(2.0f, 2.0f, 1.0f));
    
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
    
    
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gJoystickBoundsTexture).GetGLTextureId()));
    
    world = glm::mat4(1.0f);
    world = glm::translate(world, glm::vec3(b2InitPos.x, b2InitPos.y, 0.0f));
    world = glm::scale(world, glm::vec3(4.0f, 4.0f, 1.0f));
    
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
}

void renderPlayer(b2Body* body, Camera& c)
{
    auto& meshResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh);
    
    auto& shaderResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::ShaderResource>(gBasicShader);
    
    GL_CALL(glUseProgram(shaderResource.GetProgramId()));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gPlayerTexture).GetGLTextureId()));
    
    glm::mat4 world(1.0f);
    world = glm::translate(world, glm::vec3(body->GetWorldCenter().x, body->GetWorldCenter().y, 0.0f));
    
    const auto& shape = dynamic_cast<const b2PolygonShape&>(*body->GetFixtureList()->GetShape());
    
    const auto scaleX = b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x);
    const auto scaleY = b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y);
    world = glm::scale(world, glm::vec3(scaleX, scaleY, 1.0f));
    
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
    
}

void renderB2DBullet(b2Body* body, Camera& c)
{
    auto& meshResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh);
    
    auto& shaderResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::ShaderResource>(gBasicShader);
    
    GL_CALL(glUseProgram(shaderResource.GetProgramId()));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gBulletTexture).GetGLTextureId()));
    
    glm::mat4 world(1.0f);
    world = glm::translate(world, glm::vec3(body->GetWorldCenter().x, body->GetWorldCenter().y, 0.0f));
    
    const auto& shape = dynamic_cast<const b2PolygonShape&>(*body->GetFixtureList()->GetShape());
    
    const auto scaleX = b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x);
    const auto scaleY = b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y);
    world = glm::scale(world, glm::vec3(scaleX, scaleY, 1.0f));
    
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
}

void renderB2DBox(b2Body* body, Camera& c)
{
    auto& meshResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh);
    
    auto& shaderResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::ShaderResource>(gBasicShader);
    
    GL_CALL(glUseProgram(shaderResource.GetProgramId()));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gEnemyTexture).GetGLTextureId()));
    
    glm::mat4 world(1.0f);
    world = glm::translate(world, glm::vec3(body->GetWorldCenter().x, body->GetWorldCenter().y, 0.0f));
    
    const auto& shape = dynamic_cast<const b2PolygonShape&>(*body->GetFixtureList()->GetShape());
    
    const auto scaleX = b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x);
    const auto scaleY = b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y);
    world = glm::scale(world, glm::vec3(scaleX, scaleY, 1.0f));
    
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
}

void renderInvisibleWall(b2Body* body, Camera& c)
{
    auto& meshResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh);
    
    auto& shaderResource = resources::ResourceLoadingService::GetInstance().GetResource<resources::ShaderResource>(gBasicShader);
    
    GL_CALL(glUseProgram(shaderResource.GetProgramId()));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gInvisWallTexture).GetGLTextureId()));
    
    glm::mat4 world(1.0f);
    world = glm::translate(world, glm::vec3(body->GetWorldCenter().x, body->GetWorldCenter().y, 0.0f));
    
    const auto& shape = dynamic_cast<const b2PolygonShape&>(*body->GetFixtureList()->GetShape());
    
    const auto scaleX = b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x);
    const auto scaleY = b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y);
    world = glm::scale(world, glm::vec3(scaleX, scaleY, 1.0f));
    
    shaderResource.SetMatrix4fv(StringId("world"), world);
    shaderResource.SetMatrix4fv(StringId("view"), c.GetViewMatrix());
    shaderResource.SetMatrix4fv(StringId("proj"), c.GetProjMatrix());
    
    GL_CALL(glDrawElements(GL_TRIANGLES, meshResource.GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
}


static constexpr uint16 ENEMY_CATEGORY_BIT  = 0x1;
static constexpr uint16 BULLET_CATEGORY_BIT = 0x2;
static constexpr uint16 PLAYER_CATEGORY_BIT = 0x4;
static constexpr uint16 WALL_CATEGORY_BIT = 0x8;

b2Body* createPlayer(b2World* world, float x = 0.0f, float y = -10.0f)
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
    fixtureDef.friction = 0.9f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = PLAYER_CATEGORY_BIT;
    fixtureDef.filter.maskBits &= (~PLAYER_CATEGORY_BIT);
    body->CreateFixture(&fixtureDef);
    
    return body;
}

b2Body* createBullet(b2World* world, b2Body* player)
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
    fixtureDef.friction = 0.9f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = BULLET_CATEGORY_BIT;
    fixtureDef.filter.maskBits &= (~PLAYER_CATEGORY_BIT);
    fixtureDef.filter.maskBits &= (~BULLET_CATEGORY_BIT);
    
    body->CreateFixture(&fixtureDef);
    
    return body;
}

b2Body* createBox(b2World* world)
{
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(math::RandomFloat(-5.0f, 5.0f), math::RandomFloat(12.0f, 16.0f));
    b2Body* body = world->CreateBody(&bodyDef);
    body->SetLinearDamping(10.0f);
    
    b2PolygonShape dynamicBox;
    
    auto& enemyTexture = resources::ResourceLoadingService::GetInstance().GetResource<resources::TextureResource>(gEnemyTexture);
    
    float textureAspect = enemyTexture.GetDimensions().x/enemyTexture.GetDimensions().y;
    dynamicBox.SetAsBox(1.0f, 1.0f/textureAspect);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.9f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = ENEMY_CATEGORY_BIT;
    body->CreateFixture(&fixtureDef);
    
    health[body] = 2;
    
    return body;
}

#define L_WALL
#define R_WALL
#define B_WALL
void Game::Run()
{
    b2World world(b2Vec2(0.0f, 0.0f));
    
    class ContactListener : public b2ContactListener
    {
    public:
        void BeginContact(b2Contact* contact) override
        {
            const auto catA = contact->GetFixtureA()->GetFilterData().categoryBits;
            const auto catB = contact->GetFixtureB()->GetFilterData().categoryBits;
            
            if (catA == ENEMY_CATEGORY_BIT && catB == BULLET_CATEGORY_BIT)
            {
                
                if (health[contact->GetFixtureA()->GetBody()] == 1)
                {
                    bodiesToRemove.insert(contact->GetFixtureA()->GetBody());
                }
                else
                {
                    health[contact->GetFixtureA()->GetBody()]--;
                }
                
                bodiesToRemove.insert(contact->GetFixtureB()->GetBody());
            }
            else if (catA == BULLET_CATEGORY_BIT && catB == ENEMY_CATEGORY_BIT)
            {
                if (health[contact->GetFixtureB()->GetBody()] == 1)
                {
                    bodiesToRemove.insert(contact->GetFixtureB()->GetBody());
                }
                else
                {
                    health[contact->GetFixtureB()->GetBody()]--;

                }
                
                bodiesToRemove.insert(contact->GetFixtureA()->GetBody());
            }
        }
    
        std::unordered_set<b2Body*> bodiesToRemove;
    };
    
    ContactListener cl;
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
    
    std::unordered_set<b2Body*> boxes;
    std::unordered_set<b2Body*> bullets;
    std::unordered_set<b2Body*> walls;
    
    
    for (int i = 0; i < 10; ++i)
        boxes.insert(createBox(&world));
    b2Body* player = createPlayer(&world);
    

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
        fixtureDef.friction = 0.9f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        walls.insert(wallBody);
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
        fixtureDef.friction = 0.9f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        walls.insert(wallBody);
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
        fixtureDef.friction = 0.9f;
        fixtureDef.restitution = 0.0f;
        fixtureDef.filter.categoryBits = WALL_CATEGORY_BIT;
     
        wallBody->CreateFixture(&fixtureDef);
        
        walls.insert(wallBody);
    }
    
    
#endif
    
    std::vector<DelayedFlow> flows;
    //int spawnCount = 0;
    flows.emplace_back([&]()
    {
    bullets.insert(createBullet(&world, player));
    }, 300.0f, DelayedFlow::RepeatPolicy::REPEAT);
    
    flows.emplace_back([&]()
    {
        boxes.insert(createBox(&world));
    }, 600.0f, DelayedFlow::RepeatPolicy::REPEAT);
    
    //float timeStep = 1.0f / 60.0f;
    int velocityIterations = 6;
    int positionIterations = 2;
    float timeStep = 1.0f / 60.0f;
    SDL_Event e;
    
    auto lastFrameMilisSinceInit = 0.0f;
    auto secsAccumulator         = 0.0f;
    auto framesAccumulator       = 0LL;
    bool playerMotion            = false;
    glm::vec2 playerMotionInitPos;
    glm::vec2 joyStickPos;
    
    std::unique_ptr<b2Vec2> attractionPoint = nullptr;
    
    
    //While application is running
    while(!mIsFinished)
    {
        // Calculate frame delta
        const auto currentMilisSinceInit = static_cast<float>(SDL_GetTicks());        // the number of milliseconds since the SDL library
        const auto dtMilis = currentMilisSinceInit - lastFrameMilisSinceInit; // milis diff between current and last frame
        
        lastFrameMilisSinceInit = currentMilisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMilis * 0.001f; // dt in seconds;
        
        static float msAccum = 0.0f;
        msAccum -= dtMilis/40000.0f;
        
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
                {
                    playerMotion = true;
                    playerMotionInitPos = math::ComputeTouchCoordsInWorldSpace(glm::vec2(windowWidth, windowHeight), glm::vec2(e.tfinger.x, e.tfinger.y), guiCam.GetViewMatrix(), guiCam.GetProjMatrix());
                    joyStickPos = playerMotionInitPos;
                    printf("SDL_FINGERDOWN: %.2f, %.2f\n", playerMotionInitPos.x, playerMotionInitPos.y);
                } break;
                case SDL_FINGERUP:
                {
                    playerMotion = false;
                    player->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
                    player->SetAwake(false);
                } break;
                case SDL_FINGERMOTION:
                {
                    auto motionVec = math::ComputeTouchCoordsInWorldSpace(glm::vec2(windowWidth, windowHeight), glm::vec2(e.tfinger.x, e.tfinger.y), guiCam.GetViewMatrix(), guiCam.GetProjMatrix()) - playerMotionInitPos;
                    
                    glm::vec2 norm = glm::normalize(motionVec);
                    if (glm::length(motionVec) > glm::length(norm))
                    {
                        motionVec = norm;
                    }
                    
                    joyStickPos = playerMotionInitPos + motionVec;

                    //motionVec.Normalize();
                    motionVec.x *= 4.0f * dtMilis/10;
                    motionVec.y *= 4.0f * dtMilis/10;
                    player->SetLinearVelocity(b2Vec2(motionVec.x, motionVec.y));

                } break;
                default: break;
            }
        }
        
        if (secsAccumulator > 1.0f)
        {
            Log(LogType::INFO, "FPS: %d", framesAccumulator);
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
        }
        
        //scalingFactor -= 0.001 * interFrameMilis;
        
        attractionPoint = std::make_unique<b2Vec2>(player->GetWorldCenter());
        if (attractionPoint)
        {
            for (auto* body: boxes)
            {
                b2Vec2 toAttractionPoint = *attractionPoint - body->GetWorldCenter();

                if (toAttractionPoint.Length() < 0.5f)
                {
                    body->SetAwake(false);
                }
                else
                {
                    toAttractionPoint.Normalize();
                    toAttractionPoint.x *= 30.0f * dtMilis/10;
                    toAttractionPoint.y *= 30.0f * dtMilis/10;
                    body->ApplyForceToCenter(toAttractionPoint, true);
                }
            }
        }
        
    
        for (auto& flow: flows) flow.update(dtMilis);
        
        world.Step(timeStep, velocityIterations, positionIterations);
  
        for (auto* body: cl.bodiesToRemove)
        {
            bullets.erase(body);
            boxes.erase(body);
            health.erase(body);
            world.DestroyBody(body);
            
        }
        cl.bodiesToRemove.clear();
        
        
        // Set View Port
        GL_CALL(glViewport(0, 0, windowWidth, windowHeight));
           
        // Set background color
        GL_CALL(glClearColor
        (
            0x11/255.0f,
            0x13/255.0f,
            0x22/255.0f,
            1.0f
        ));

        
        // Clear buffers
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
        
        
        GL_CALL(glBindVertexArray(resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(gMesh).GetVertexArrayObject()));
        
        renderSpaceBackground(msAccum, guiCam);
        
        //renderB2DBox(groundBody);
        bool log = false;
        for (auto* body: boxes)
        {
            renderB2DBox(body, cam);
            log = false;
        }
        
        for (auto* body: bullets)
        {
            renderB2DBullet(body, cam);
            log = false;
        }
        
        renderPlayer(player, cam);
        
        for (auto* body: walls)
        {
            renderInvisibleWall(body, cam);
        }
        
        if (playerMotion)
        {
            renderJoystick(b2Vec2(joyStickPos.x, joyStickPos.y), b2Vec2(playerMotionInitPos.x, playerMotionInitPos.y), guiCam);
        }
        
        // Swap window buffers
        GL_CALL(SDL_GL_SwapWindow(window));
    }
}

///------------------------------------------------------------------------------------------------
