///------------------------------------------------------------------------------------------------
///  SceneObjectUtils.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "SceneObjectUtils.h"
#include "Scene.h"
#include "datarepos/FontRepository.h"
#include "definitions/ObjectTypeDefinition.h"
#include "PhysicsConstants.h"
#include "../resloading/TextureResource.h"
#include "../resloading/MeshResource.h"
#include "../utils/OSMessageBox.h"
#include <Box2D/Box2D.h>

///------------------------------------------------------------------------------------------------

namespace scene_object_utils
{

///------------------------------------------------------------------------------------------------

static const std::string BOSS_SCENE_OBJECT_NAME_PREFIX = "enemies/boss";

///------------------------------------------------------------------------------------------------

static std::unordered_map<char, Glyph>::const_iterator GetGlyphIter(char c, const FontDefinition& fontDef)
{
    auto findIter = fontDef.mGlyphs.find(c);
    if (findIter == fontDef.mGlyphs.end())
    {
        return fontDef.mGlyphs.find(' ');
    }
    return findIter;
}

///------------------------------------------------------------------------------------------------

void GetSceneObjectBoundingRect(const SceneObject& sceneObject, glm::vec2& rectBotLeft, glm::vec2& rectTopRight)
{
    // Text SO
    if (!sceneObject.mText.empty())
    {
        auto fontOpt = FontRepository::GetInstance().GetFont(sceneObject.mFontName);
        if (!fontOpt) return;
        
        const auto& font = fontOpt->get();
        
        float xCursor = sceneObject.mPosition.x;
        float yCursor = sceneObject.mPosition.y;
        float minX = xCursor;
        float minY = yCursor;
        float maxX = xCursor;
        float maxY = yCursor;
        
        for (size_t i = 0; i < sceneObject.mText.size(); ++i)
        {
            const auto& glyph = GetGlyphIter(sceneObject.mText[i], font)->second;
            
            float targetX = xCursor;
            float targetY = yCursor + glyph.mYOffsetPixels * sceneObject.mScale.y * 0.5f;
            
            if (targetX + glyph.mWidthPixels * sceneObject.mScale.x/2 > maxX) maxX = targetX + glyph.mWidthPixels * sceneObject.mScale.x/2;
            if (targetX - glyph.mWidthPixels * sceneObject.mScale.x/2 < minX) minX = targetX - glyph.mWidthPixels * sceneObject.mScale.x/2;
            if (targetY + glyph.mHeightPixels * sceneObject.mScale.y/2 > maxY) maxY = targetY + glyph.mHeightPixels * sceneObject.mScale.y/2;
            if (targetY - glyph.mHeightPixels * sceneObject.mScale.y/2 < minY) minY = targetY - glyph.mHeightPixels * sceneObject.mScale.y/2;
            
            if (i != sceneObject.mText.size() - 1)
            {
                // Since each glyph is rendered with its center as the origin, we advance
                // half this glyph's width + half the next glyph's width ahead
                const auto& nextGlyph = GetGlyphIter(sceneObject.mText[i + 1], font)->second;
                xCursor += (glyph.mWidthPixels * sceneObject.mScale.x) * 0.5f + (nextGlyph.mWidthPixels * sceneObject.mScale.x) * 0.5f;
            }
        }
        
        rectBotLeft = glm::vec2(minX, minY);
        rectTopRight = glm::vec2(maxX, maxY);
    }
    // SO with Physical Body
    else if (sceneObject.mBody)
    {
        const auto& shape = dynamic_cast<const b2PolygonShape&>(*sceneObject.mBody->GetFixtureList()->GetShape());
        
        auto soPosition = glm::vec3(sceneObject.mBody->GetWorldCenter().x, sceneObject.mBody->GetWorldCenter().y, sceneObject.mPosition.z);
        auto soScale = glm::vec3(b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x), b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y), 1.0f);
        
        auto rectBotLeft = glm::vec2(soPosition.x - soScale.x/2, soPosition.y - soScale.y/2);
        auto rectTopRight = glm::vec2(soPosition.x + soScale.x/2, soPosition.y + soScale.y/2);
    }
    // SO with custom position and scale
    else
    {
        rectBotLeft = glm::vec2(sceneObject.mPosition.x - math::Abs(sceneObject.mScale.x/2), sceneObject.mPosition.y - math::Abs(sceneObject.mScale.y/2));
        rectTopRight = glm::vec2(sceneObject.mPosition.x + math::Abs(sceneObject.mScale.x/2), sceneObject.mPosition.y + math::Abs(sceneObject.mScale.y/2));
    }
}

///------------------------------------------------------------------------------------------------

bool IsPointInsideSceneObject(const SceneObject& sceneObject, const glm::vec2& point, const glm::vec2 xyBias /* glm::vec2(1.0f, 1.0f) */)
{
    glm::vec2 rectBotLeft, rectTopRight;
    GetSceneObjectBoundingRect(sceneObject, rectBotLeft, rectTopRight);
    
    // Text SO
    if (!sceneObject.mText.empty())
    {
        rectBotLeft.x *= xyBias.x;
        rectBotLeft.y *= xyBias.y;
        
        rectTopRight.x *= xyBias.x;
        rectTopRight.y *= xyBias.y;
    }
    // SO with Physical Body or custom position and scale
    else
    {
        rectBotLeft.x += math::Abs(sceneObject.mScale.x/2);
        rectBotLeft.x -= math::Abs(sceneObject.mScale.x/2) * xyBias.x;
        
        rectBotLeft.y += math::Abs(sceneObject.mScale.y/2);
        rectBotLeft.y -= math::Abs(sceneObject.mScale.y/2) * xyBias.y;
        
        rectTopRight.x -= math::Abs(sceneObject.mScale.x/2);
        rectTopRight.x += math::Abs(sceneObject.mScale.x/2) * xyBias.x;
        
        rectTopRight.y -= math::Abs(sceneObject.mScale.y/2);
        rectTopRight.y += math::Abs(sceneObject.mScale.y/2) * xyBias.y;
    }
    
    return math::IsPointInsideRectangle(rectBotLeft, rectTopRight, point);
}

///------------------------------------------------------------------------------------------------

void ChangeSceneObjectState(SceneObject& sceneObject, const ObjectTypeDefinition& objectDef, const strutils::StringId newStateName)
{
    sceneObject.mStateName = newStateName;
    
    auto animationIter = objectDef.mAnimations.find(newStateName);
    if (animationIter != objectDef.mAnimations.end())
    {
        sceneObject.mAnimation = animationIter->second->VClone();
        
        if (!sceneObject.mAnimation->VGetBodyRenderingEnabled() && sceneObject.mBody)
        {
            sceneObject.mPosition.x = sceneObject.mBody->GetWorldCenter().x;
            sceneObject.mPosition.y = sceneObject.mBody->GetWorldCenter().y;
            sceneObject.mScale = sceneObject.mAnimation->VGetScale();
            auto filter = sceneObject.mBody->GetFixtureList()[0].GetFilterData();
            filter.maskBits = 0;
            sceneObject.mBody->GetFixtureList()[0].SetFilterData(filter);
        }
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Invalid state transition", ("State name " + newStateName.GetString() + " for object type" + objectDef.mName.GetString() + " was not found!").c_str());
    }
}

///------------------------------------------------------------------------------------------------

bool IsSceneObjectBossPart(const SceneObject& sceneObject)
{
    return strutils::StringStartsWith(sceneObject.mName.GetString(), BOSS_SCENE_OBJECT_NAME_PREFIX);
}

///------------------------------------------------------------------------------------------------

strutils::StringId GenerateSceneObjectName(const SceneObject& sceneObject)
{
    if (!sceneObject.mBody) return strutils::StringId();
    
    strutils::StringId name;
    name.fromAddress(sceneObject.mBody);
    return name;
}

///------------------------------------------------------------------------------------------------

SceneObject CreateSceneObjectWithBody(const ObjectTypeDefinition& objectDef, const glm::vec3& position, b2World& box2dWorld, strutils::StringId sceneObjectName)
{
    SceneObject so;
    so.mAnimation = objectDef.mAnimations.at(game_constants::DEFAULT_SCENE_OBJECT_STATE)->VClone();
    
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    
    so.mStateName = game_constants::DEFAULT_SCENE_OBJECT_STATE;
    so.mBodyCustomOffset = objectDef.mBodyCustomOffset;
    so.mBodyCustomScale = objectDef.mBodyCustomScale;
    
    bodyDef.position.Set(position.x + objectDef.mBodyCustomOffset.x, position.y + objectDef.mBodyCustomOffset.y);
    
    b2Body* body = box2dWorld.CreateBody(&bodyDef);
    body->SetLinearDamping(objectDef.mLinearDamping);
    
    b2PolygonShape dynamicBox;
    auto& mesh = resources::ResourceLoadingService::GetInstance().GetResource<resources::MeshResource>(so.mAnimation->VGetCurrentMeshResourceId());
    
    dynamicBox.SetAsBox((mesh.GetDimensions().x * math::Abs(so.mAnimation->VGetScale().x) * math::Abs(objectDef.mBodyCustomScale.x))/2, (mesh.GetDimensions().y * math::Abs(so.mAnimation->VGetScale().y) * math::Abs(objectDef.mBodyCustomScale.y))/2);
    
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.filter = objectDef.mContactFilter;
    fixtureDef.density = objectDef.mDensity;
    
    body->CreateFixture(&fixtureDef);
    
    so.mObjectFamilyTypeName = objectDef.mName;
    so.mBody = body;
    so.mHealth = objectDef.mHealth;
    so.mSceneObjectType = SceneObjectType::WorldGameObject;
    so.mScale = so.mAnimation->VGetScale();
    
    so.mPosition.z = position.z;
    so.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = true;
    
    if (sceneObjectName.isEmpty())
    {
        so.mName = GenerateSceneObjectName(so);
    }
    else
    {
        so.mName = sceneObjectName;
    }
    
    so.mBody->SetUserData(new strutils::StringId(so.mName));
    
    return so;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
