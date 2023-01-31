///------------------------------------------------------------------------------------------------
///  GameObjectDefinitionLoader.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/01/2023                                                       
///------------------------------------------------------------------------------------------------

#include "GameObjectDefinitionLoader.h"
#include "../PhysicsConstants.h"
#include "../../resloading/ResourceLoadingService.h"
#include "../../utils/Logging.h"

#include <rapidxml/rapidxml.hpp>

///------------------------------------------------------------------------------------------------

GameObjectDefinitionLoader::GameObjectDefinitionLoader()
{
    /*
     <Physics density="1.0f" bodySize="1.0f" linearDamping="10.0f" linearSpeed="0.0f, 20.0f" category="enemy" collidingWithPlayerBullets="true" collidingWithEnemies="true" collidingWithPlayer="true"/>
     <Shader name="basic"/>
     <Model name="quad"/>
     <Texture name="enemy"/>
     <GameAttributes movementPattern="vertical" health="5"/>
     */
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Physics"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* density = node->first_attribute("density");
        if (density)
        {
            mConstructedGODef.mDensity = std::stof(density->value());
        }
        
        auto* bodySize = node->first_attribute("bodySize");
        if (bodySize)
        {
            mConstructedGODef.mSize = std::stof(bodySize->value());
        }
        
        auto* linearDamping = node->first_attribute("linearDamping");
        if (linearDamping)
        {
            mConstructedGODef.mLinearDamping = std::stof(linearDamping->value());
        }
        
        auto* linearSpeed = node->first_attribute("speed");
        if (linearSpeed)
        {
            mConstructedGODef.mSpeed = std::stof(linearSpeed->value());
        }
        
        auto* customLinearVelocity = node->first_attribute("customLinearVelocity");
        if (customLinearVelocity)
        {
            auto customLinearVelocityComponents = strutils::StringSplit(std::string(customLinearVelocity->value()), ',');
            mConstructedGODef.mCustomLinearVelocity.x = std::stof(customLinearVelocityComponents[0]);
            mConstructedGODef.mCustomLinearVelocity.y = std::stof(customLinearVelocityComponents[1]);
        }

        auto* category = node->first_attribute("category");
        if (category)
        {
            if (strcmp(category->value(), "enemy") == 0)
            {
                mConstructedGODef.mContactFilter.categoryBits = ENEMY_CATEGORY_BIT;
            }
            else if (strcmp(category->value(), "player") == 0)
            {
                mConstructedGODef.mContactFilter.categoryBits = PLAYER_CATEGORY_BIT;
            }
        }
        
        auto* shouldCollideWithPlayerBullets = node->first_attribute("collidingWithPlayerBullets");
        if (shouldCollideWithPlayerBullets && strcmp(shouldCollideWithPlayerBullets->value(), "false") == 0)
        {
            mConstructedGODef.mContactFilter.maskBits &= (~PLAYER_BULLET_CATEGORY_BIT);
        }
        
        auto* shouldCollideWithEnemyBullets = node->first_attribute("collidingWithEnemyBullets");
        if (shouldCollideWithEnemyBullets && strcmp(shouldCollideWithEnemyBullets->value(), "false") == 0)
        {
            mConstructedGODef.mContactFilter.maskBits &= (~ENEMY_BULLET_CATEGORY_BIT);
        }
        
        auto* shouldCollideWithEnemies = node->first_attribute("collidingWithEnemies");
        if (shouldCollideWithEnemies && strcmp(shouldCollideWithEnemies->value(), "false") == 0)
        {
            mConstructedGODef.mContactFilter.maskBits &= (~ENEMY_CATEGORY_BIT);
        }
        
        auto* shouldCollideWithPlayer = node->first_attribute("collidingWithPlayer");
        if (shouldCollideWithPlayer && strcmp(shouldCollideWithPlayer->value(), "false") == 0)
        {
            mConstructedGODef.mContactFilter.maskBits &= (~PLAYER_CATEGORY_BIT);
        }
    });

    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Shader"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedGODef.mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + std::string(name->value()) + ".vs");
        }
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Model"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedGODef.mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MODELS_ROOT + std::string(name->value()) + ".obj");
        }
    });
    
    BaseGameDataLoader::SetCallbackForNode(strutils::StringId("Texture"), [&](const void* n)
    {
        auto* node = static_cast<const rapidxml::xml_node<>*>(n);
        
        auto* name = node->first_attribute("name");
        if (name)
        {
            mConstructedGODef.mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::string(name->value()) + ".bmp");
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
                mConstructedGODef.mMovementControllerPattern = MovementControllerPattern::CUSTOM_VELOCITY;
            }
            else if (strcmp(movementControllerPattern->value(), "chasing_player") == 0)
            {
                mConstructedGODef.mMovementControllerPattern = MovementControllerPattern::CHASING_PLAYER;
            }
            else if (strcmp(movementControllerPattern->value(), "input_controlled") == 0)
            {
                mConstructedGODef.mMovementControllerPattern = MovementControllerPattern::INPUT_CONTROLLED;
            }
        }
        
        auto* health = node->first_attribute("health");
        if (health)
        {
            mConstructedGODef.mHealth = std::stoi(health->value());
        }
    });
}

///------------------------------------------------------------------------------------------------

GameObjectDefinition& GameObjectDefinitionLoader::LoadGameObjectDefinition(const std::string &gameObjectDefinitionFileName)
{
    mConstructedGODef = GameObjectDefinition();
    mConstructedGODef.mName = strutils::StringId(gameObjectDefinitionFileName);
    
    BaseGameDataLoader::LoadData(gameObjectDefinitionFileName);
    
    return mConstructedGODef;
}

///------------------------------------------------------------------------------------------------
