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

ObjectTypeDefinitionLoader::ObjectTypeDefinitionLoader()
{
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Physics"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* density = node->first_attribute("density");
        if (density)
        {
            mConstructedObjectTypeDef.mDensity = std::stof(density->value());
        }
        
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
        
        auto* constantLinearVelocity = node->first_attribute("constantLinearVelocity");
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
            }
            else if (strcmp(category->value(), "player") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::PLAYER_CATEGORY_BIT;
            }
            else if (strcmp(category->value(), "player_bullet") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::PLAYER_BULLET_CATEGORY_BIT;
            }
            else if (strcmp(category->value(), "enemy_bullet") == 0)
            {
                mConstructedObjectTypeDef.mContactFilter.categoryBits = physics_constants::ENEMY_BULLET_CATEGORY_BIT;
            }
        }
        
        auto* shouldCollideWithPlayerBullets = node->first_attribute("collidingWithPlayerBullets");
        if (shouldCollideWithPlayerBullets && strcmp(shouldCollideWithPlayerBullets->value(), "false") == 0)
        {
            mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_BULLET_CATEGORY_BIT);
        }
        
        auto* shouldCollideWithEnemyBullets = node->first_attribute("collidingWithEnemyBullets");
        if (shouldCollideWithEnemyBullets && strcmp(shouldCollideWithEnemyBullets->value(), "false") == 0)
        {
            mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::ENEMY_BULLET_CATEGORY_BIT);
        }
        
        auto* shouldCollideWithEnemies = node->first_attribute("collidingWithEnemies");
        if (shouldCollideWithEnemies && strcmp(shouldCollideWithEnemies->value(), "false") == 0)
        {
            mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::ENEMY_CATEGORY_BIT);
        }
        
        auto* shouldCollideWithPlayer = node->first_attribute("collidingWithPlayer");
        if (shouldCollideWithPlayer && strcmp(shouldCollideWithPlayer->value(), "false") == 0)
        {
            mConstructedObjectTypeDef.mContactFilter.maskBits &= (~physics_constants::PLAYER_CATEGORY_BIT);
        }
    });

    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Shader"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedObjectTypeDef.mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + std::string(name->value()) + ".vs");
        }
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Model"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedObjectTypeDef.mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + std::string(name->value()) + ".obj");
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
                
                animation = new VariableTexturedAnimation(potentialTextureResourceIds);
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
                animation = new MultiFrameAnimation(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(texture->value()) + ".bmp"), duration, scale, textureSheetRow);
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
                animation = new DissolveAnimation(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(texture->value()) + ".bmp"), dissolveTextureResourceId, dissolveSpeed);
            }
        }
        
        // Single Frame Animation
        if (!animation)
        {
            auto* texture = node->first_attribute("texture");
            if (texture)
            {
                animation = new SingleFrameAnimation(resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(texture->value()) + ".bmp"));
            }
        }
        
        auto* state = node->first_attribute("state");
        if (state)
        {
            mConstructedObjectTypeDef.mAnimations[strutils::StringId(state->value())] = animation;
        }
    });
    
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Texture"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedObjectTypeDef.mAnimations[scene_object_constants::DEFAULT_SCENE_OBJECT_STATE] = new SingleFrameAnimation( resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(name->value()) + ".bmp"));
        }
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("GameAttributes"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* movementControllerPattern = node->first_attribute("movementControllerPattern");
        if (movementControllerPattern)
        {
            if (strcmp(movementControllerPattern->value(), "custom_velocity") == 0)
            {
                mConstructedObjectTypeDef.mMovementControllerPattern = MovementControllerPattern::CUSTOM_VELOCITY;
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
