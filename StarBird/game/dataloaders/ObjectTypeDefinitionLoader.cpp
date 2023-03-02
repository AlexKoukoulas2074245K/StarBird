///------------------------------------------------------------------------------------------------
///  ObjectTypeDefinitionLoader.cpp
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "ObjectTypeDefinitionLoader.h"
#include "../PhysicsConstants.h"
#include "../SceneObjectConstants.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/Logging.h"
#include "../../utils/MathUtils.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

resources::ResourceId LoadTexture(const rapidxml::xml_node<>* node)
{
    auto* texture = node->first_attribute("texture");
    if (texture)
    {
        return resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(texture->value()) + ".bmp");
    }
    else
    {
        return resources::ResourceLoadingService::FALLBACK_TEXTURE_ID;
    }
}

///------------------------------------------------------------------------------------------------

resources::ResourceId LoadMesh(const rapidxml::xml_node<>* node)
{
    auto* mesh = node->first_attribute("mesh");
    if (mesh)
    {
        return resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + std::string(mesh->value()) + ".obj");
    }
    else
    {
        return resources::ResourceLoadingService::FALLBACK_MESH_ID;
    }
}

///------------------------------------------------------------------------------------------------

resources::ResourceId LoadShader(const rapidxml::xml_node<>* node)
{
    auto* shader = node->first_attribute("shader");
    if (shader)
    {
        return resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + std::string(shader->value()) + ".vs");
    }
    else
    {
        return resources::ResourceLoadingService::FALLBACK_SHADER_ID;
    }
}

///------------------------------------------------------------------------------------------------

ObjectTypeDefinitionLoader::ObjectTypeDefinitionLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Physics"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* bodySize = node->first_attribute("bodySize");
        if (bodySize)
        {
            mConstructedObjectTypeDef.mSize = std::stof(bodySize->value());
        }
        
        auto* linearDamping = node->first_attribute("linearDamping");
        if (linearDamping)
        {
            mConstructedObjectTypeDef.mLinearDamping = std::stof(linearDamping->value());
        }
        
        auto* linearSpeed = node->first_attribute("speed");
        if (linearSpeed)
        {
            mConstructedObjectTypeDef.mSpeed = std::stof(linearSpeed->value());
        }
        
        auto* constantLinearVelocity = node->first_attribute("constantVelocity");
        if (constantLinearVelocity)
        {
            auto constantLinearVelocityComponents = strutils::StringSplit(std::string(constantLinearVelocity->value()), ',');
            mConstructedObjectTypeDef.mConstantLinearVelocity.x = std::stof(constantLinearVelocityComponents[0]);
            mConstructedObjectTypeDef.mConstantLinearVelocity.y = std::stof(constantLinearVelocityComponents[1]);
        }

        auto* category = node->first_attribute("category");
        if (category)
        {
            if (strcmp(category->value(), "enemy") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::ENEMY_CATEGORY_BIT;
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT);
            }
            else if (strcmp(category->value(), "player") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::PLAYER_CATEGORY_BIT;
                mConstructedObjectTypeDef.mContactFilter.maskBits &= ~(physics_constants::PLAYER_BULLET_CATEGORY_BIT);
            }
            else if (strcmp(category->value(), "player_bullet") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::PLAYER_BULLET_CATEGORY_BIT;
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_BULLET_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::ENEMY_BULLET_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_CATEGORY_BIT);
            }
            else if (strcmp(category->value(), "enemy_bullet") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::ENEMY_BULLET_CATEGORY_BIT;
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::ENEMY_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_BULLET_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::ENEMY_BULLET_CATEGORY_BIT);
            }
            else if (strcmp(category->value(), "boss") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::ENEMY_CATEGORY_BIT;
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::ENEMY_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::BULLET_ONLY_WALL_CATEGORY_BIT);
                mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_ONLY_WALL_CATEGORY_BIT);
            }
        }
    });

    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Animation"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        IAnimation* animation = nullptr;
        
        // Variable Textured Animation
        auto* textureText = node->first_attribute("texture");
        if (textureText)
        {
            std::string textureName(textureText->value());
            if (textureName.back() == '}')
            {
                auto textureNameSplitByLBrace = strutils::StringSplit(textureName, '{');
                auto textureRangeStr = textureNameSplitByLBrace[1];
                textureRangeStr.pop_back();
                
                auto rangeComponents = strutils::StringSplit(textureRangeStr, ':');
                auto minTextureNumber = std::stoi(rangeComponents[0]);
                auto maxTextureNumber = std::stoi(rangeComponents[1]);
                
                std::vector<resources::ResourceId> potentialTextureResourceIds;
                
                for (int i = minTextureNumber; i <= maxTextureNumber; ++i)
                {
                    potentialTextureResourceIds.push_back(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + textureNameSplitByLBrace[0] + std::to_string(i) + ".bmp"));
                }
                
                animation = new VariableTexturedAnimation(potentialTextureResourceIds, LoadMesh(node), LoadShader(node));
            }
        }
            
        // Multi Frame Animation
        auto* textureSheetRowText = node->first_attribute("textureSheetRow");
        if (textureSheetRowText)
        {
            int textureSheetRow = std::stoi(textureSheetRowText->value());
            float duration = 0.0f;
            float scale = 1.0f;
            
            auto* durationText = node->first_attribute("duration");
            if (durationText)
            {
                duration = std::stof(durationText->value());
            }
            
            auto* scaleText = node->first_attribute("scale");
            if (scaleText)
            {
                scale = std::stof(scaleText->value());
            }
            
            auto* texture = node->first_attribute("texture");
            if (texture)
            {
                animation = new MultiFrameAnimation(LoadTexture(node), LoadMesh(node), LoadShader(node), duration, scale, textureSheetRow);
            }
        }
        
        // Dissolve Animation
        auto* dissolveTextureText = node->first_attribute("dissolveTexture");
        if (dissolveTextureText)
        {
            resources::ResourceId dissolveTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + dissolveTextureText->value() + ".bmp");
            float dissolveSpeed = 0.0f;
            
            auto* dissolveSpeedText = node->first_attribute("dissolveSpeed");
            if (dissolveSpeedText)
            {
                dissolveSpeed = std::stof(dissolveSpeedText->value());
            }
            
            auto* texture = node->first_attribute("texture");
            if (texture)
            {
                animation = new DissolveAnimation(nullptr, LoadTexture(node), dissolveTextureResourceId, LoadMesh(node), LoadShader(node), dissolveSpeed);
            }
        }
        
        // Single Frame Animation
        if (!animation)
        {
            auto* texture = node->first_attribute("texture");
            if (texture)
            {
                animation = new SingleFrameAnimation(LoadTexture(node), LoadMesh(node), LoadShader(node));
            }
        }
        
        auto* state = node->first_attribute("state");
        if (state)
        {
            mConstructedObjectTypeDef.mAnimations[strutils::StringId(state->value())] = animation;
        }
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("GameAttributes"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* movementControllerPattern = node->first_attribute("movementControllerPattern");
        if (movementControllerPattern)
        {
            if (strcmp(movementControllerPattern->value(), "constant_velocity") == 0)
            {
                mConstructedObjectTypeDef.mMovementControllerPattern = MovementControllerPattern::CONSTANT_VELOCITY;
            }
            else if (strcmp(movementControllerPattern->value(), "chasing_player") == 0)
            {
                mConstructedObjectTypeDef.mMovementControllerPattern = MovementControllerPattern::CHASING_PLAYER;
            }
            else if (strcmp(movementControllerPattern->value(), "input_controlled") == 0)
            {
                mConstructedObjectTypeDef.mMovementControllerPattern = MovementControllerPattern::INPUT_CONTROLLED;
            }
        }
        
        auto* health = node->first_attribute("health");
        if (health)
        {
            mConstructedObjectTypeDef.mHealth = std::stof(health->value());
        }
        
        auto* damage = node->first_attribute("damage");
        if (damage)
        {
            mConstructedObjectTypeDef.mDamage = std::stof(damage->value());
        }
        
        auto* shootingFrequency = node->first_attribute("shootingFrequency");
        if (shootingFrequency)
        {
            mConstructedObjectTypeDef.mShootingFrequencyMillis = std::stof(shootingFrequency->value());
        }
        
        auto* projectile = node->first_attribute("projectile");
        if (projectile)
        {
            mConstructedObjectTypeDef.mProjectileType = strutils::StringId(projectile->value());
            mSubObjectsFound->insert(mConstructedObjectTypeDef.mProjectileType);
        }
        
        auto* flipped = node->first_attribute("flipped");
        if (flipped)
        {
            if (strcmp(flipped->value(), "x") == 0)
            {
                mConstructedObjectTypeDef.mFlippedDisplay = FlippedDisplay::FLIPPED_X;
            }
            else if (strcmp(flipped->value(), "y") == 0)
            {
                mConstructedObjectTypeDef.mFlippedDisplay = FlippedDisplay::FLIPPED_Y;
            }
            else if (strcmp(flipped->value(), "xy") == 0)
            {
                mConstructedObjectTypeDef.mFlippedDisplay = FlippedDisplay::FLIPPED_XY;
            }
        }
    });
}

///------------------------------------------------------------------------------------------------

ObjectTypeDefinition&& ObjectTypeDefinitionLoader::LoadObjectTypeDefinition(const std::string& objectTypeDefinitionFileName, std::unordered_set<strutils::StringId, strutils::StringIdHasher>* subObjectsFound)
{
    mConstructedObjectTypeDef = ObjectTypeDefinition();
    mConstructedObjectTypeDef.mName = strutils::StringId(objectTypeDefinitionFileName);
    mSubObjectsFound = subObjectsFound;
    
    BaseGameDataLoader::LoadData(objectTypeDefinitionFileName);
    
    return std::move(mConstructedObjectTypeDef);
}

///------------------------------------------------------------------------------------------------
