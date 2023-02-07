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
        
        auto* customLinearVelocity = node->first_attribute("customLinearVelocity");
        if (customLinearVelocity)
        {
            auto customLinearVelocityComponents = strutils::StringSplit(std::string(customLinearVelocity->value()), ',');
            mConstructedObjectTypeDef.mCustomLinearVelocity.x = std::stof(customLinearVelocityComponents[0]);
            mConstructedObjectTypeDef.mCustomLinearVelocity.y = std::stof(customLinearVelocityComponents[1]);
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
        
        Animation animation;
        
        auto* texture = node->first_attribute("texture");
        if (texture)
        {
            animation.mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(texture->value()) + ".bmp");
        }
        
        auto* duration = node->first_attribute("duration");
        if (duration)
        {
            animation.mDuration = std::stof(duration->value());
        }
        
        auto* scale = node->first_attribute("scale");
        if (scale)
        {
            animation.mScale = std::stof(scale->value());
        }
        
        auto* textureSheetRow = node->first_attribute("textureSheetRow");
        if (textureSheetRow)
        {
            animation.mTextureSheetRow = std::stoi(textureSheetRow->value());
        }
        
        auto* state = node->first_attribute("state");
        if (state)
        {
            mConstructedObjectTypeDef.mAnimations[strutils::StringId(state->value())] = std::move(animation);
        }
    });
    
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Texture"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedObjectTypeDef.mAnimations[sceneobject_constants::DEFAULT_SCENE_OBJECT_STATE].mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(name->value()) + ".bmp");
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
            mConstructedObjectTypeDef.mHealth = std::stoi(health->value());
        }
        
        auto* damage = node->first_attribute("damage");
        if (damage)
        {
            mConstructedObjectTypeDef.mDamage = std::stoi(damage->value());
        }
    });
}

///------------------------------------------------------------------------------------------------

ObjectTypeDefinition& ObjectTypeDefinitionLoader::LoadObjectTypeDefinition(const std::string &objectTypeDefinitionFileName)
{
    mConstructedObjectTypeDef = ObjectTypeDefinition();
    mConstructedObjectTypeDef.mName = strutils::StringId(objectTypeDefinitionFileName);
    
    BaseGameDataLoader::LoadData(objectTypeDefinitionFileName);
    
    return mConstructedObjectTypeDef;
}

///------------------------------------------------------------------------------------------------
