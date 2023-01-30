///------------------------------------------------------------------------------------------------
///  SceneRenderer.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/01/2023                                                       
///------------------------------------------------------------------------------------------------

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
    auto* window = SDL_GL_GetCurrentWindow();
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    mSceneObjectTypeToCamera[SceneObjectType::WorldGameObject] = Camera(windowWidth, windowHeight, 30.0f);
    mSceneObjectTypeToCamera[SceneObjectType::GUIGameObject] = Camera(windowWidth, windowHeight, 30.0f);
}

///------------------------------------------------------------------------------------------------

void SceneRenderer::Render(const std::vector<SceneObject>& sceneObjects)
{
    auto& resService = resources::ResourceLoadingService::GetInstance();
    
    auto* window = SDL_GL_GetCurrentWindow();
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    
    // Set View Port
    GL_CALL(glViewport(0, 0, windowWidth, windowHeight));
       
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
        
        if (so.mTextureResourceId != currentTextureResourceId)
        {
            currentTextureResourceId = so.mTextureResourceId;
            GL_CALL(glBindTexture(GL_TEXTURE_2D, resService.GetResource<resources::TextureResource>(currentTextureResourceId).GetGLTextureId()));
        }
        
        glm::mat4 world(1.0f);
        
        // If a b2Body is active then take its transform
        if (so.mBody)
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
        
        currentShader->SetMatrix4fv(WORLD_MATRIX_STRING_ID, world);
        currentShader->SetMatrix4fv(VIEW_MATRIX_STRING_ID, mSceneObjectTypeToCamera.at(so.mSceneObjectType).GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_STRING_ID, mSceneObjectTypeToCamera.at(so.mSceneObjectType).GetProjMatrix());
        
        for (auto floatEntry: so.mShaderFloatUniformValues)
            currentShader->SetFloat(floatEntry.first, floatEntry.second);
        for (auto mat4Entry: so.mShaderMat4UniformValues)
            currentShader->SetMatrix4fv(mat4Entry.first, mat4Entry.second);
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
    }
    
    // Swap window buffers
    GL_CALL(SDL_GL_SwapWindow(window));
}

///------------------------------------------------------------------------------------------------
