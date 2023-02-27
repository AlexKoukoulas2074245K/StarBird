///------------------------------------------------------------------------------------------------
///  SceneRenderer.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneRenderer_h
#define SceneRenderer_h

///------------------------------------------------------------------------------------------------

#include "Camera.h"
#include "SceneObject.h"

#include <set>
#include <Box2D/Common/b2Draw.h>

///------------------------------------------------------------------------------------------------
class b2World;
class SceneRenderer final: public b2Draw
{
public:
    SceneRenderer(b2World& box2dWorld);
    
    void Render(std::vector<SceneObject>& sceneObjects);
    
    void SetPhysicsDebugMode(const bool physicsDebugMode);
    
    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {};
    void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {};
    void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {};
    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {};
    void DrawTransform(const b2Transform& xf) {};
    
private:
    b2World& mBox2dWorld;
    bool mPhysicsDebugMode;
    std::vector<std::array<b2Vec2, 4>> mPhysicsDebugQuads;
};

///------------------------------------------------------------------------------------------------

#endif /* SceneRenderer_h */
