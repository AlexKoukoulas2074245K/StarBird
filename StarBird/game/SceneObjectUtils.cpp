///------------------------------------------------------------------------------------------------
///  SceneObjectUtils.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "SceneObjectUtils.h"
#include "Scene.h"
#include "datarepos/FontRepository.h"

///------------------------------------------------------------------------------------------------

namespace scene_object_utils
{

///------------------------------------------------------------------------------------------------

bool IsPointInsideSceneObject(const SceneObject& sceneObject, const glm::vec2& point)
{
    // Text SO
    if (!sceneObject.mText.empty())
    {
        auto fontOpt = FontRepository::GetInstance().GetFont(sceneObject.mFontName);
        if (!fontOpt) return false;
        
        const auto& font = fontOpt->get();
        
        float xCursor = sceneObject.mCustomPosition.x;
        float yCursor = sceneObject.mCustomPosition.y;
        float minX = xCursor;
        float minY = yCursor;
        float maxX = xCursor;
        float maxY = yCursor;
        
        for (size_t i = 0; i < sceneObject.mText.size(); ++i)
        {
            const auto& glyph = font.mGlyphs.at(sceneObject.mText[i]);
            
            float targetX = xCursor;
            float targetY = yCursor + glyph.mYOffsetPixels * sceneObject.mCustomScale.y * 0.5f;
            
            if (targetX + glyph.mWidthPixels * sceneObject.mCustomScale.x/2 > maxX) maxX = targetX + glyph.mWidthPixels * sceneObject.mCustomScale.x/2;
            if (targetX - glyph.mWidthPixels * sceneObject.mCustomScale.x/2 < minX) minX = targetX - glyph.mWidthPixels * sceneObject.mCustomScale.x/2;
            if (targetY + glyph.mHeightPixels * sceneObject.mCustomScale.y/2 > maxY) maxY = targetY + glyph.mHeightPixels * sceneObject.mCustomScale.y/2;
            if (targetY - glyph.mHeightPixels * sceneObject.mCustomScale.y/2 < minY) minY = targetY - glyph.mHeightPixels * sceneObject.mCustomScale.y/2;
            
            if (i != sceneObject.mText.size() - 1)
            {
                // Since each glyph is rendered with its center as the origin, we advance
                // half this glyph's width + half the next glyph's width ahead
                const auto& nextGlyph = font.mGlyphs.at(sceneObject.mText[i + 1]);
                xCursor += (glyph.mWidthPixels * sceneObject.mCustomScale.x) * 0.5f + (nextGlyph.mWidthPixels * sceneObject.mCustomScale.x) * 0.5f;
            }
        }
        
        auto rectBottomLeft = glm::vec2(minX, minY);
        auto rectTopRight = glm::vec2(maxX, maxY);
        
        return math::IsPointInsideRectangle(rectBottomLeft, rectTopRight, point);
        
        return true;
    }
    // SO with Physical Body
    else if (sceneObject.mBody)
    {
        const auto& shape = dynamic_cast<const b2PolygonShape&>(*sceneObject.mBody->GetFixtureList()->GetShape());
        
        auto soPosition = glm::vec3(sceneObject.mBody->GetWorldCenter().x, sceneObject.mBody->GetWorldCenter().y, sceneObject.mCustomPosition.z);
        auto soScale = glm::vec3(b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x), b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y), 1.0f);
        
        auto rectBottomLeft = glm::vec2(soPosition.x - soScale.x/2, soPosition.y - soScale.y/2);
        auto rectTopRight = glm::vec2(soPosition.x + soScale.x/2, soPosition.y + soScale.y/2);
        
        return math::IsPointInsideRectangle(rectBottomLeft, rectTopRight, point);
    }
    // SO with custom position and scale
    else
    {
        auto rectBottomLeft = glm::vec2(sceneObject.mCustomPosition.x - sceneObject.mCustomScale.x/2, sceneObject.mCustomPosition.y - sceneObject.mCustomScale.y/2);
        auto rectTopRight = glm::vec2(sceneObject.mCustomPosition.x + sceneObject.mCustomScale.x/2, sceneObject.mCustomPosition.y + sceneObject.mCustomScale.y/2);
        
        return math::IsPointInsideRectangle(rectBottomLeft, rectTopRight, point);
    }
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
