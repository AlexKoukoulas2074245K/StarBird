///------------------------------------------------------------------------------------------------
///  TextPromptController.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/03/2023                                                       
///------------------------------------------------------------------------------------------------

#include "Animations.h"
#include "FontRepository.h"
#include "GameConstants.h"
#include "Scene.h"
#include "SceneObject.h"
#include "TextPromptController.h"

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

TextPromptController::TextPromptController(Scene& scene, std::unique_ptr<BaseAnimation> animation, const glm::vec3& position, const glm::vec3& scale, bool fadeIn, const std::string& text)
    : mScene(scene)
{
    // Text prompt background
    SceneObject textPromptSo;
    textPromptSo.mPosition = position;
    textPromptSo.mScale = scale;
    textPromptSo.mAnimation = std::move(animation);
    textPromptSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    textPromptSo.mName = game_constants::TEXT_PROMPT_NAME;
    textPromptSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = fadeIn ? 0.0f : 1.0f;
    mSceneObjectNamesToTransparencyDelayMillis[textPromptSo.mName] = 0.0f;
    
    mScene.AddSceneObject(std::move(textPromptSo));
    
    auto fontOpt = FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME);
    if (fontOpt)
    {
        const auto& font = fontOpt->get();
        
        const float glyphScale = ((scale.x + scale.y)/2.0f)/1428.5714f;
        const float leftBoundMagic = 0.43f;
        const float rightBoundMagic = 0.41f;
        const float yOffsetMagic = 0.25f;
        const float wrapYMagic = 100.0f;
        
        float xCursor = position.x - scale.x * leftBoundMagic;
        float xCutoff = position.x + scale.x * rightBoundMagic;
        float yCursor = position.y + scale.y * yOffsetMagic;
        
        auto words = strutils::StringSplit(text, ' ');
        for (size_t i = 0; i < words.size() - 1; ++i)
        {
            words[i] += " ";
        }
        
        auto charCounter = 0;
        for (size_t i = 0; i < words.size(); ++i)
        {
            // Look forward to see if word fits in current line
            auto cursorCopy = xCursor;
            auto currentWord = words[i];
            for (size_t j = 0; j < currentWord.size() - 1; ++j)
            {
                const auto& glyph = GetGlyphIter(currentWord[j], font)->second;
                const auto& nextGlyph = GetGlyphIter(currentWord[j + 1], font)->second;
                
                cursorCopy += (glyph.mWidthPixels * glyphScale) * 0.5f + (nextGlyph.mWidthPixels * glyphScale) * 0.5f;
                if (cursorCopy > xCutoff)
                {
                    xCursor = position.x - scale.x * leftBoundMagic;
                    yCursor -= glyphScale * wrapYMagic;
                    break;
                }
            }
            
            for (size_t j = 0; j < currentWord.size(); ++j)
            {
                const auto& glyph = GetGlyphIter(currentWord[j], font)->second;
                auto& resService = resources::ResourceLoadingService::GetInstance();
                
                SceneObject glyphSO;
                glyphSO.mPosition = glm::vec3(xCursor, yCursor, position.z + 0.5f);
                glyphSO.mScale = glm::vec3(glyphScale, glyphScale, position.z + 0.5f);
                glyphSO.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(1.0f), false);
                glyphSO.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = fadeIn ? 0.0f : 1.0f;
                glyphSO.mFontName = game_constants::DEFAULT_FONT_NAME;
                glyphSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                glyphSO.mName = strutils::StringId(std::to_string(charCounter));
                glyphSO.mText = std::string(1, currentWord[j]);
                mSceneObjectNamesToTransparencyDelayMillis[glyphSO.mName] = charCounter * 20.0f;
                mScene.AddSceneObject(std::move(glyphSO));
                
                if (i != words.size() - 1 || j != currentWord.size() - 1)
                {
                    // Since each glyph is rendered with its center as the origin, we advance
                    // half this glyph's width + half the next glyph's width ahead
                    const auto& nextGlyph = GetGlyphIter(currentWord[j + 1], font)->second;
                    xCursor += (glyph.mWidthPixels * glyphScale) * 0.5f + (nextGlyph.mWidthPixels * glyphScale) * 0.5f;
                }
                
                charCounter++;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

TextPromptController::~TextPromptController()
{
    for (const auto& iter: mSceneObjectNamesToTransparencyDelayMillis)
    {
        mScene.RemoveAllSceneObjectsWithName(iter.first);
    }
}

///------------------------------------------------------------------------------------------------

void TextPromptController::Update(const float dtMillis)
{
    for (auto& iter: mSceneObjectNamesToTransparencyDelayMillis)
    {
        if (iter.second > 0)
        {
            iter.second -= dtMillis;
        }
        else
        {
            auto soOpt = mScene.GetSceneObject(iter.first);
            if (soOpt)
            {
                auto& sceneObject = soOpt->get();
                sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * game_constants::TEXT_FADE_IN_ALPHA_SPEED;
                if (sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= 1.0f)
                {
                    sceneObject.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                }
            }
        }
    }
}

///------------------------------------------------------------------------------------------------
