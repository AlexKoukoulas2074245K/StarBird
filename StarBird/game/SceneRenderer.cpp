///------------------------------------------------------------------------------------------------
///  SceneRenderer.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "FontRepository.h"
#include "GameSingletons.h"
#include "SceneObjectConstants.h"
#include "SceneRenderer.h"

#include "../resloading/MeshResource.h"
#include "../resloading/ShaderResource.h"
#include "../resloading/TextureResource.h"
#include "../utils/Logging.h"
#include "../utils/OpenGL.h"

#include <Box2D/Box2D.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId WORLD_MATRIX_STRING_ID = strutils::StringId("world");
static const strutils::StringId VIEW_MATRIX_STRING_ID  = strutils::StringId("view");
static const strutils::StringId PROJ_MATRIX_STRING_ID  = strutils::StringId("proj");

///------------------------------------------------------------------------------------------------

SceneRenderer::SceneRenderer()
{
}

///------------------------------------------------------------------------------------------------

void SceneRenderer::Render(std::vector<SceneObject>& sceneObjects)
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
        
        if (so.mMeshResourceId != currentMeshReourceId)
        {
            currentMeshReourceId = so.mMeshResourceId;
            currentMesh = &(resService.GetResource<resources::MeshResource>(currentMeshReourceId));
            GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        }
        
        if (so.mShaderResourceId != currentShaderResourceId)
        {
            currentShaderResourceId = so.mShaderResourceId;
            currentShader = &(resService.GetResource<resources::ShaderResource>(currentShaderResourceId));
            GL_CALL(glUseProgram(currentShader->GetProgramId()));
        }
        
        if (so.mTextureResourceId == 0 || so.mTextureResourceId != currentTextureResourceId)
        {
            currentTextureResourceId = so.mTextureResourceId;
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(currentTextureResourceId).GetGLTextureId()));
        }
        
        if (so.mShaderUniformTextureResourceId != 0)
        {
            GL_CALL(glActiveTexture(GL_TEXTURE1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(so.mShaderUniformTextureResourceId).GetGLTextureId()));
        }
        
        glm::mat4 world(1.0f);
        
        // If it's a text element
        if (so.mText.size() > 0)
        {
            auto fontOpt = FontRepository::GetInstance().GetFont(so.mFontName);
            if (fontOpt)
            {
                const auto& font = fontOpt->get();
                
                const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(so.mSceneObjectType);
                assert(camOpt);
                const auto& camera = camOpt->get();
                
                float xCursor = so.mCustomPosition.x;
                float yCursor = so.mCustomPosition.y;
                
                for (size_t i = 0; i < so.mText.size(); ++i)
                {
                    const auto& glyph = font.mGlyphs.at(so.mText[i]);
                    
                    float targetX = xCursor;
                    float targetY = yCursor + glyph.mYOffsetPixels * so.mCustomScale.y * 0.5f;
                    
                    world = glm::mat4(1.0f);
                    world = glm::translate(world, glm::vec3(targetX, targetY, so.mCustomPosition.z));
                    world = glm::scale(world, glm::vec3(glyph.mWidthPixels * so.mCustomScale.x, glyph.mHeightPixels * so.mCustomScale.y, 1.0f));
                    
                    so.mShaderBoolUniformValues[scene_object_constants::IS_TEXTURE_SHEET_UNIFORM_NAME] = true;
                    so.mShaderFloatUniformValues[scene_object_constants::MIN_U_UNIFORM_NAME] = glyph.minU;
                    so.mShaderFloatUniformValues[scene_object_constants::MIN_V_UNIFORM_NAME] = glyph.minV;
                    so.mShaderFloatUniformValues[scene_object_constants::MAX_U_UNIFORM_NAME] = glyph.maxU;
                    so.mShaderFloatUniformValues[scene_object_constants::MAX_V_UNIFORM_NAME] = glyph.maxV;
                    
                    currentShader->SetMatrix4fv(WORLD_MATRIX_STRING_ID, world);
                    currentShader->SetMatrix4fv(VIEW_MATRIX_STRING_ID, camera.GetViewMatrix());
                    currentShader->SetMatrix4fv(PROJ_MATRIX_STRING_ID, camera.GetProjMatrix());
                    
                    for (auto boolEntry: so.mShaderBoolUniformValues)
                        currentShader->SetBool(boolEntry.first, boolEntry.second);
                    for (auto intEntry: so.mShaderIntUniformValues)
                        currentShader->SetInt(intEntry.first, intEntry.second);
                    for (auto floatEntry: so.mShaderFloatUniformValues)
                        currentShader->SetFloat(floatEntry.first, floatEntry.second);
                    for (auto mat4Entry: so.mShaderMat4UniformValues)
                        currentShader->SetMatrix4fv(mat4Entry.first, mat4Entry.second);
                    
                    GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
                    
                    if (i != so.mText.size() - 1)
                    {
                        // Since each glyph is rendered with its center as the origin, we advance
                        // half this glyph's width + half the next glyph's width ahead
                        const auto& nextGlyph = font.mGlyphs.at(so.mText[i + 1]);
                        xCursor += (glyph.mWidthPixels * so.mCustomScale.x) * 0.5f + (nextGlyph.mWidthPixels * so.mCustomScale.x) * 0.5f;
                    }
                }
            }
        }
        // If a b2Body is active then take its transform
        else if (so.mBody && so.mUseBodyForRendering)
        {
            world = glm::translate(world, glm::vec3(so.mBody->GetWorldCenter().x, so.mBody->GetWorldCenter().y, so.mCustomPosition.z));
            
            //world = glm::rotate(world, so.mCustomRotation);
            const auto& shape = dynamic_cast<const b2PolygonShape&>(*so.mBody->GetFixtureList()->GetShape());
            
            const auto scaleX = b2Abs(shape.GetVertex(1).x - shape.GetVertex(3).x);
            const auto scaleY = b2Abs(shape.GetVertex(1).y - shape.GetVertex(3).y);
            world = glm::scale(world, glm::vec3(scaleX, scaleY, 1.0f));
        }
        // Otherwise from its custom set one
        else
        {
            world = glm::translate(world, so.mCustomPosition);
            //world = glm::rotate(world, so.mCustomRotation);
            world = glm::scale(world, so.mCustomScale);
        }
        
        const auto& camOpt = GameSingletons::GetCameraForSceneObjectType(so.mSceneObjectType);
        assert(camOpt);
        const auto& camera = camOpt->get();
        
        currentShader->SetMatrix4fv(WORLD_MATRIX_STRING_ID, world);
        currentShader->SetMatrix4fv(VIEW_MATRIX_STRING_ID, camera.GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_STRING_ID, camera.GetProjMatrix());
        
        for (auto boolEntry: so.mShaderBoolUniformValues)
            currentShader->SetBool(boolEntry.first, boolEntry.second);
        for (auto intEntry: so.mShaderIntUniformValues)
            currentShader->SetInt(intEntry.first, intEntry.second);
        for (auto floatEntry: so.mShaderFloatUniformValues)
            currentShader->SetFloat(floatEntry.first, floatEntry.second);
        for (auto mat4Entry: so.mShaderMat4UniformValues)
            currentShader->SetMatrix4fv(mat4Entry.first, mat4Entry.second);
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
    }
    
    // Swap window buffers
    GL_CALL(SDL_GL_SwapWindow(GameSingletons::GetWindow()));
}

///------------------------------------------------------------------------------------------------
