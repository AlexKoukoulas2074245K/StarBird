///------------------------------------------------------------------------------------------------
///  SceneRenderer.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameSingletons.h"
#include "GameConstants.h"
#include "SceneObjectUtils.h"
#include "SceneRenderer.h"

#include "datarepos/LightRepository.h"

#include "../resloading/MeshResource.h"
#include "../resloading/ShaderResource.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"
#include "../utils/OpenGL.h"

#include <Box2D/Box2D.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const char* EDIT_MODE_SELECTED_SO_OUTLINE_TEXTURE_FILE_NAME = "edit_mode_selection_outline.bmp";

static const strutils::StringId WORLD_MATRIX_UNIFORM_NAME = strutils::StringId("world");
static const strutils::StringId VIEW_MATRIX_UNIFORM_NAME  = strutils::StringId("view");
static const strutils::StringId PROJ_MATRIX_UNIFORM_NAME  = strutils::StringId("proj");
static const strutils::StringId ACTIVE_LIGHT_COUNT_UNIFORM_NAME = strutils::StringId("active_light_count");
static const strutils::StringId AMBIENT_LIGHT_COLOR_UNIFORM_NAME = strutils::StringId("ambient_light_color");
static const strutils::StringId POINT_LIGHT_COLORS_UNIFORM_NAME = strutils::StringId("point_light_colors");
static const strutils::StringId POINT_LIGHT_POSITIONS_UNIFORM_NAME = strutils::StringId("point_light_positions");
static const strutils::StringId POINT_LIGHT_POWERS_UNIFORM_NAME = strutils::StringId("point_light_powers");
static const glm::vec4 DEBUG_VERTEX_COLOR = glm::vec4(0.0f, 0.0f, 1.0f, 0.5f);
static const float DEBUG_VERTEX_Z = 3.0f;
static const float DEBUG_VERTEX_ASPECT_SCALE = 1.2f;

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

SceneRenderer::SceneRenderer(b2World& box2dWorld)
    : mBox2dWorld(box2dWorld)
    , mPhysicsDebugMode(false)
{
    mBox2dWorld.SetDebugDraw(this);
    SetFlags(b2Draw::e_aabbBit);
    resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME);
}

///------------------------------------------------------------------------------------------------

void SceneRenderer::Render(std::vector<SceneObject>& sceneObjects, const LightRepository& lightRepository)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    const auto& windowDimensions = GameSingletons::GetWindowDimensions();
    
    // Set View Port
    GL_CALL(glViewport(0, 0, windowDimensions.x, windowDimensions.y));
       
    // Set background color
    GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    
    // Clear buffers
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    // Reusability optimisation
    resources::ResourceId currentMeshReourceId = resources::ResourceId();
    resources::ResourceId currentShaderResourceId = resources::ResourceId();
    resources::ResourceId currentTextureResourceId = resources::ResourceId();
    resources::MeshResource* currentMesh = nullptr;
    resources::ShaderResource* currentShader = nullptr;
    
    for (auto& so: sceneObjects)
    {
        if (so.mInvisible) continue;
        
        assert(so.mAnimation);
        
        if (so.mAnimation->VGetCurrentMeshResourceId() != currentMeshReourceId)
        {
            currentMeshReourceId = so.mAnimation->VGetCurrentMeshResourceId();
            currentMesh = &(resService.GetResource<resources::MeshResource>(currentMeshReourceId));
            GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        }
        
        if (so.mAnimation->VGetCurrentShaderResourceId() != currentShaderResourceId)
        {
            currentShaderResourceId = so.mAnimation->VGetCurrentShaderResourceId();
            currentShader = &(resService.GetResource<resources::ShaderResource>(currentShaderResourceId));
            GL_CALL(glUseProgram(currentShader->GetProgramId()));
        }
        
        for (size_t i = 0; i < currentShader->GetUniformSamplerNames().size(); ++i)
        {
            currentShader->SetInt(currentShader->GetUniformSamplerNames().at(i), static_cast<int>(i));
        }
        
        if (so.mShaderBoolUniformValues.count(game_constants::IS_TEXTURE_SHEET_UNIFORM_NAME) == 0)
        {
            so.mShaderBoolUniformValues[game_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = false;
        }
        
        if (so.mShaderBoolUniformValues.count(game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME) == 0)
        {
            so.mShaderBoolUniformValues[game_constants::IS_AFFECTED_BY_LIGHT_UNIFORM_NAME] = false;
        }
        
        
        if (so.mAnimation->VGetCurrentTextureResourceId() == 0 || so.mAnimation->VGetCurrentTextureResourceId() != currentTextureResourceId)
        {
            currentTextureResourceId = so.mAnimation->VGetCurrentTextureResourceId();
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(currentTextureResourceId).GetGLTextureId()));
        }
        
        if (so.mAnimation->VGetCurrentEffectTextureResourceId() != 0)
        {
            GL_CALL(glActiveTexture(GL_TEXTURE1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(so.mAnimation->VGetCurrentEffectTextureResourceId()).GetGLTextureId()));
        }
        
        glm::mat4 world(1.0f);
        
        // If it's a text element
        if (!so.mFontName.isEmpty() && so.mText.size() > 0)
        {
            bool usesCustomColor = so.mAnimation->VGetCurrentShaderResourceId() == resService.GetResourceIdFromPath(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME);
            
            auto fontOpt = FontRepository::GetInstance().GetFont(so.mFontName);
            if (fontOpt)
            {
                const auto& font = fontOpt->get();
                
                const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(so.mSceneObjectType);
                assert(camOpt);
                const auto& camera = camOpt->get();
                
                float xCursor = so.mPosition.x;
                float yCursor = so.mPosition.y;
                
                for (size_t i = 0; i < so.mText.size(); ++i)
                {
                    const auto& glyph = GetGlyphIter(so.mText[i], font)->second;
                    
                    float targetX = xCursor;
                    float targetY = yCursor + glyph.mYOffsetPixels * so.mScale.y * 0.5f;
                    
                    world = glm::mat4(1.0f);
                    world = glm::translate(world, glm::vec3(targetX, targetY, so.mPosition.z));
                    world = glm::scale(world, glm::vec3(glyph.mWidthPixels * so.mScale.x, glyph.mHeightPixels * so.mScale.y, 1.0f));
                    
                    so.mShaderBoolUniformValues[game_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
                    so.mShaderFloatUniformValues[game_constants::MIN_U_UNIFORM_NAME] = glyph.minU;
                    so.mShaderFloatUniformValues[game_constants::MIN_V_UNIFORM_NAME] = glyph.minV;
                    so.mShaderFloatUniformValues[game_constants::MAX_U_UNIFORM_NAME] = glyph.maxU;
                    so.mShaderFloatUniformValues[game_constants::MAX_V_UNIFORM_NAME] = glyph.maxV;
                    
                    currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
                    currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, camera.GetViewMatrix());
                    currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, camera.GetProjMatrix());
                    
                    for (const auto& boolEntry: so.mShaderBoolUniformValues)
                        currentShader->SetBool(boolEntry.first, boolEntry.second);
                    for (const auto& intEntry: so.mShaderIntUniformValues)
                        currentShader->SetInt(intEntry.first, intEntry.second);
                    for (const auto& floatEntry: so.mShaderFloatUniformValues)
                        currentShader->SetFloat(floatEntry.first, floatEntry.second);
                    for (const auto& floatVec4Entry: so.mShaderFloatVec4UniformValues)
                        currentShader->SetFloatVec4(floatVec4Entry.first, floatVec4Entry.second);
                    for (const auto& mat4Entry: so.mShaderMat4UniformValues)
                        currentShader->SetMatrix4fv(mat4Entry.first, mat4Entry.second);
                    
                    GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
                    
                    if (i != so.mText.size() - 1)
                    {
                        // Since each glyph is rendered with its center as the origin, we advance
                        // half this glyph's width + half the next glyph's width ahead
                        const auto& nextGlyph = GetGlyphIter(so.mText[i + 1], font)->second;
                        
                        const auto additionalSpacingMultiplier = usesCustomColor ? 1.5f : 1.0f;
                        xCursor += (glyph.mWidthPixels * so.mScale.x) * 0.5f + (nextGlyph.mWidthPixels * so.mScale.x) * 0.5f * additionalSpacingMultiplier;
                    }
                }
            }
        }
        // If a b2Body is active then take its transform
        else if (so.mBody && so.mAnimation->VGetBodyRenderingEnabled())
        {
            world = glm::translate(world, math::Box2dVec2ToGlmVec3(so.mBody->GetWorldCenter(), so.mPosition.z) - so.mBodyCustomOffset);
            
            world = glm::rotate(world, so.mRotation.x, math::X_AXIS);
            world = glm::rotate(world, so.mRotation.y, math::Y_AXIS);
            world = glm::rotate(world, so.mRotation.z, math::Z_AXIS);
           
            world = glm::scale(world, so.mScale);
        }
        // Otherwise from its custom set one
        else
        {
            world = glm::translate(world, so.mPosition);
            world = glm::rotate(world, so.mRotation.x, math::X_AXIS);
            world = glm::rotate(world, so.mRotation.y, math::Y_AXIS);
            world = glm::rotate(world, so.mRotation.z, math::Z_AXIS);
            world = glm::scale(world, so.mScale);
        }
        
        const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(so.mSceneObjectType);
        assert(camOpt);
        const auto& camera = camOpt->get();
        
        currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
        currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, camera.GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, camera.GetProjMatrix());
        currentShader->SetFloatVec4(AMBIENT_LIGHT_COLOR_UNIFORM_NAME, lightRepository.mAmbientLightColor);
        currentShader->SetFloatVec4Array(POINT_LIGHT_COLORS_UNIFORM_NAME, lightRepository.mPointLightColors);
        currentShader->SetFloatVec3Array(POINT_LIGHT_POSITIONS_UNIFORM_NAME, lightRepository.mPointLightPositions);
        currentShader->SetFloatArray(POINT_LIGHT_POWERS_UNIFORM_NAME, lightRepository.mPointLightPowers);
        currentShader->SetInt(ACTIVE_LIGHT_COUNT_UNIFORM_NAME, static_cast<int>(lightRepository.mPointLightNames.size()));
        
        for (const auto& boolEntry: so.mShaderBoolUniformValues)
            currentShader->SetBool(boolEntry.first, boolEntry.second);
        for (const auto& intEntry: so.mShaderIntUniformValues)
            currentShader->SetInt(intEntry.first, intEntry.second);
        for (const auto& floatEntry: so.mShaderFloatUniformValues)
            currentShader->SetFloat(floatEntry.first, floatEntry.second);
        for (const auto& floatVec4Entry: so.mShaderFloatVec4UniformValues)
            currentShader->SetFloatVec4(floatVec4Entry.first, floatVec4Entry.second);
        for (const auto& mat4Entry: so.mShaderMat4UniformValues)
            currentShader->SetMatrix4fv(mat4Entry.first, mat4Entry.second);
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
        
        if (so.mDebugEditSelected)
        {
            currentMesh = &(resService.GetResource<resources::MeshResource>(resources::ResourceLoadingService::FALLBACK_MESH_ID));
            GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
            
            currentShader = &(resService.GetResource<resources::ShaderResource>(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BASIC_SHADER_FILE_NAME));
            
            GL_CALL(glUseProgram(currentShader->GetProgramId()));
            
            currentTextureResourceId = resources::ResourceLoadingService::FALLBACK_TEXTURE_ID;
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>( resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + EDIT_MODE_SELECTED_SO_OUTLINE_TEXTURE_FILE_NAME)).GetGLTextureId()));
    
            const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(so.mSceneObjectType);
            assert(camOpt);
            const auto& camera = camOpt->get();
            
            glm::vec2 boundingRectBotLeft, boundingRectTopRight;
            scene_object_utils::GetSceneObjectBoundingRect(so, boundingRectBotLeft, boundingRectTopRight);
            
            glm::mat4 world = glm::mat4(1.0f);
            world = glm::translate(world, glm::vec3((boundingRectBotLeft.x + boundingRectTopRight.x)/2.0f, (boundingRectBotLeft.y + boundingRectTopRight.y)/2.0f, so.mPosition.z + 0.5f));
            
            world = glm::scale(world, glm::vec3(math::Abs(boundingRectBotLeft.x - boundingRectTopRight.x), math::Abs(boundingRectBotLeft.y - boundingRectTopRight.y), 1.0f));
            
            currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
            currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, camera.GetViewMatrix());
            currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, camera.GetProjMatrix());
            
            GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
        }
    }
   
    if (mPhysicsDebugMode)
    {
        mPhysicsDebugQuads.clear();
        
        // This will populate (via individual callbacks) all debug vertices from box2d's POV
        mBox2dWorld.DrawDebugData();
        
        const auto& windowDimensions = GameSingletons::GetWindowDimensions();
        float aspectFactor = windowDimensions.x/windowDimensions.y * DEBUG_VERTEX_ASPECT_SCALE;
        
        for (const auto& debugQuad: mPhysicsDebugQuads)
        {
            currentMesh = &(resService.GetResource<resources::MeshResource>(resources::ResourceLoadingService::FALLBACK_MESH_ID));
            GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
            
            currentShader = &(resService.GetResource<resources::ShaderResource>(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_COLOR_SHADER_FILE_NAME));
            
            GL_CALL(glUseProgram(currentShader->GetProgramId()));
            
            currentTextureResourceId = resources::ResourceLoadingService::FALLBACK_TEXTURE_ID;
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(currentTextureResourceId).GetGLTextureId()));
            
            GL_CALL(glActiveTexture(GL_TEXTURE1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(currentTextureResourceId).GetGLTextureId()));
    
            const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(SceneObjectType::WorldGameObject);
            assert(camOpt);
            const auto& camera = camOpt->get();
            
            glm::mat4 world = glm::mat4(1.0f);
            float posX = debugQuad[0].x + debugQuad[1].x;
            float posY = debugQuad[1].y + debugQuad[2].y;
            world = glm::translate(world, glm::vec3(posX, posY, DEBUG_VERTEX_Z));
            
            const auto scaleX = b2Abs(debugQuad[0].x - debugQuad[1].x);
            const auto scaleY = b2Abs(debugQuad[1].y - debugQuad[2].y);
            world = glm::scale(world, glm::vec3(scaleX/aspectFactor, scaleY/aspectFactor, 1.0f));
            
            currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
            currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, camera.GetViewMatrix());
            currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, camera.GetProjMatrix());
            
            currentShader->SetFloatVec4(game_constants::CUSTOM_COLOR_UNIFORM_NAME, DEBUG_VERTEX_COLOR);
            
            GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
        }
    }
    
    // Swap window buffers
    GL_CALL(SDL_GL_SwapWindow(GameSingletons::GetWindow()));
}

///------------------------------------------------------------------------------------------------

void SceneRenderer::DrawPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color)
{
    mPhysicsDebugQuads.emplace_back();
    mPhysicsDebugQuads.back()[0] = b2Vec2(vertices[0].x/2.0f, vertices[0].y/2.0f);
    mPhysicsDebugQuads.back()[1] = b2Vec2(vertices[1].x/2.0f, vertices[1].y/2.0f);
    mPhysicsDebugQuads.back()[2] = b2Vec2(vertices[2].x/2.0f, vertices[2].y/2.0f);
    mPhysicsDebugQuads.back()[3] = b2Vec2(vertices[3].x/2.0f, vertices[3].y/2.0f);
}

///------------------------------------------------------------------------------------------------

void SceneRenderer::SetPhysicsDebugMode(const bool physicsDebugMode)
{
    mPhysicsDebugMode = physicsDebugMode;
}

///------------------------------------------------------------------------------------------------
