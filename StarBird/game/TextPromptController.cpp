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

static const float GLYPH_SCALE_INFERENCE_MAGIC = 1350.0f;
static const float LEFT_BOUND_MAGIC = 0.43f;
static const float RIGHT_BOUND_MAGIC = 0.41f;
static const float TEXT_WRAP_Y_OFFSET_MAGIC = 100.0f;
static const float INIT_Y_OFFSET_MAGIC = 0.25f;
static const float FADE_IN_DELAY_MULTIPLIER_MILLIS = 20.0f;

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

TextPromptController::TextPromptController(Scene& scene, std::unique_ptr<BaseAnimation> textPromptAreaAnimation, const glm::vec3& position, const glm::vec3& scale, bool fadeIn, const std::string& text)
    : mScene(scene)
{
    // Text prompt background
    SceneObject textPromptSo;
    textPromptSo.mPosition = position;
    textPromptSo.mScale = scale;
    textPromptSo.mAnimation = std::move(textPromptAreaAnimation);
    textPromptSo.mSceneObjectType = SceneObjectType::WorldGameObject;
    textPromptSo.mName = game_constants::TEXT_PROMPT_NAME;
    textPromptSo.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    mSceneObjectNamesToTransparencyDelayMillis[textPromptSo.mName] = 0.0f;
    
    mScene.AddSceneObject(std::move(textPromptSo));
    
    auto fontOpt = FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_NAME);
    if (fontOpt)
    {
        const auto& font = fontOpt->get();
        
        // Define constants
        const float glyphScale = ((scale.x + scale.y)/2.0f)/GLYPH_SCALE_INFERENCE_MAGIC;
        const float xCutoff = position.x + scale.x * RIGHT_BOUND_MAGIC;
        
        // and initial positions
        float xCursor = position.x - scale.x * LEFT_BOUND_MAGIC;
        float yCursor = position.y + scale.y * INIT_Y_OFFSET_MAGIC;
        
        // Split text to words and re-add space to the end of each word
        auto words = strutils::StringSplit(text, ' ');
        for (size_t i = 0; i < words.size() - 1; ++i)
        {
            if (strutils::StringContains(words[i], "\n"))
            {
                continue;
            }
            
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
                    xCursor = position.x - scale.x * LEFT_BOUND_MAGIC;
                    yCursor -= glyphScale * TEXT_WRAP_Y_OFFSET_MAGIC;
                    break;
                }
            }
            
            // Create a scene object for each char in the text prompt
            for (size_t j = 0; j < currentWord.size(); ++j)
            {
                if (currentWord[j] == '\n')
                {
                    xCursor = position.x - scale.x * LEFT_BOUND_MAGIC;
                    yCursor -= glyphScale * TEXT_WRAP_Y_OFFSET_MAGIC;
                    continue;
                }
                
                const auto& glyph = GetGlyphIter(currentWord[j], font)->second;
                auto& resService = resources::ResourceLoadingService::GetInstance();
                
                SceneObject glyphSO;
                glyphSO.mPosition = glm::vec3(xCursor, yCursor, position.z + 0.5f);
                glyphSO.mScale = glm::vec3(glyphScale, glyphScale, position.z + 0.5f);
                glyphSO.mAnimation = std::make_unique<SingleFrameAnimation>(FontRepository::GetInstance().GetFont(game_constants::DEFAULT_FONT_MM_NAME)->get().mFontTextureResourceId, resService.LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::QUAD_MESH_FILE_NAME), resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_FILE_NAME), glm::vec3(glyphScale), false);
                glyphSO.mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = fadeIn ? 0.0f : 1.0f;
                glyphSO.mFontName = game_constants::DEFAULT_FONT_MM_NAME;
                glyphSO.mSceneObjectType = SceneObjectType::WorldGameObject;
                glyphSO.mName = strutils::StringId(std::to_string(charCounter));
                glyphSO.mText = std::string(1, currentWord[j]);
                mSceneObjectNamesToTransparencyDelayMillis[glyphSO.mName] = charCounter * FADE_IN_DELAY_MULTIPLIER_MILLIS;
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
