///------------------------------------------------------------------------------------------------
///  BlueprintFlows.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/04/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BlueprintFlows_h
#define BlueprintFlows_h

///------------------------------------------------------------------------------------------------

#include "RepeatableFlow.h"

#include <unordered_set>
#include <vector>

///------------------------------------------------------------------------------------------------

class Scene;
class b2World;

namespace blueprint_flows
{

void CreatePlayerBulletFlow(std::vector<RepeatableFlow>& flows, Scene& scene, b2World& box2dWorld, const std::unordered_set<strutils::StringId, strutils::StringIdHasher> blacklistedUpgradeFlows = {});

}

///------------------------------------------------------------------------------------------------

#endif /* BlueprintFlows_h */
