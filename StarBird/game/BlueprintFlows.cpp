///------------------------------------------------------------------------------------------------
///  BlueprintFlows.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/04/2023                                                       
///------------------------------------------------------------------------------------------------

#include "BlueprintFlows.h"
#include "SceneObjectUtils.h"
#include "Scene.h"
#include "ObjectTypeDefinitionRepository.h"
#include "GameConstants.h"
#include "GameSingletons.h"
#include "PhysicsConstants.h"

///------------------------------------------------------------------------------------------------

namespace blueprint_flows
{

///------------------------------------------------------------------------------------------------

static void CreateBulletAtPosition(const strutils::StringId& bulletType, const glm::vec3& position, Scene& scene, b2World& box2dWorld)
{
    auto bulletDefOpt = ObjectTypeDefinitionRepository::GetInstance().GetObjectTypeDefinition(bulletType);
    if (bulletDefOpt)
    {
        auto& bulletDef = bulletDefOpt->get();
        auto bulletPos = position;
        bulletPos.z = game_constants::BULLET_Z;
        scene.AddSceneObject(scene_object_utils::CreateSceneObjectWithBody(bulletDef, position, box2dWorld));
    }
}

///------------------------------------------------------------------------------------------------

void CreatePlayerBulletFlow(std::vector<RepeatableFlow>& flows, Scene& scene, b2World& box2dWorld, const std::unordered_set<strutils::StringId, strutils::StringIdHasher> blacklistedUpgradeFlows /* = {} */)
{
    if (!flows.empty())
    {
        flows.erase(std::find_if(flows.begin(), flows.end(), [](const RepeatableFlow& flow){ return flow.GetName() == game_constants::PLAYER_BULLET_FLOW_NAME; }));
    }
    
    flows.emplace_back([&, blacklistedUpgradeFlows]()
    {
        bool hasDoubleBulletUpgrade = GameSingletons::HasEquippedUpgrade(game_constants::DOUBLE_BULLET_UGPRADE_NAME) && blacklistedUpgradeFlows.count(game_constants::DOUBLE_BULLET_UGPRADE_NAME) == 0;
        
        bool hasMirrorImageUpgrade = GameSingletons::HasEquippedUpgrade(game_constants::MIRROR_IMAGE_UGPRADE_NAME) && blacklistedUpgradeFlows.count(game_constants::MIRROR_IMAGE_UGPRADE_NAME) == 0;

        auto playerOpt = scene.GetSceneObject(game_constants::PLAYER_SCENE_OBJECT_NAME);
        if (playerOpt && GameSingletons::GetPlayerCurrentHealth() > 0.0f)
        {
            if (hasDoubleBulletUpgrade)
            {
                auto bulletPosition = math::Box2dVec2ToGlmVec3(playerOpt->get().mBody->GetWorldCenter());

                // Left Bullet
                bulletPosition.x -= game_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(game_constants::PLAYER_BULLET_TYPE, bulletPosition, scene, box2dWorld);

                // Right Bullet
                bulletPosition.x += 2 * game_constants::PLAYER_BULLET_X_OFFSET;
                CreateBulletAtPosition(game_constants::PLAYER_BULLET_TYPE, bulletPosition, scene, box2dWorld);

                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = scene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = scene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);

                    if (leftMirrorImageSoOpt)
                    {
                        auto bulletPosition = leftMirrorImageSoOpt->get().mPosition;
                        bulletPosition.x -= game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition, scene, box2dWorld);

                        bulletPosition.x += 2 * game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition, scene, box2dWorld);
                    }

                    if (rightMirrorImageSoOpt)
                    {
                        auto bulletPosition = rightMirrorImageSoOpt->get().mPosition;
                        bulletPosition.x -= game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition, scene, box2dWorld);

                        bulletPosition.x += 2 * game_constants::MIRROR_IMAGE_BULLET_X_OFFSET;
                        CreateBulletAtPosition(game_constants::MIRROR_IMAGE_BULLET_TYPE, bulletPosition, scene, box2dWorld);
                    }
                }
            }
            else
            {
                CreateBulletAtPosition(game_constants::PLAYER_BULLET_TYPE, math::Box2dVec2ToGlmVec3(playerOpt->get().mBody->GetWorldCenter()), scene, box2dWorld);

                if (hasMirrorImageUpgrade)
                {
                    auto leftMirrorImageSoOpt = scene.GetSceneObject(game_constants::LEFT_MIRROR_IMAGE_SCENE_OBJECT_NAME);
                    auto rightMirrorImageSoOpt = scene.GetSceneObject(game_constants::RIGHT_MIRROR_IMAGE_SCENE_OBJECT_NAME);

                    if (leftMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(game_constants::MIRROR_IMAGE_BULLET_TYPE, leftMirrorImageSoOpt->get().mPosition, scene, box2dWorld);
                    }

                    if (rightMirrorImageSoOpt)
                    {
                        CreateBulletAtPosition(game_constants::MIRROR_IMAGE_BULLET_TYPE, rightMirrorImageSoOpt->get().mPosition, scene, box2dWorld);
                    }
                }
            }
        }
    }, game_constants::BASE_PLAYER_BULLET_FLOW_DELAY_MILLIS / GameSingletons::GetPlayerBulletSpeedStat(), RepeatableFlow::RepeatPolicy::REPEAT, game_constants::PLAYER_BULLET_FLOW_NAME);
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
