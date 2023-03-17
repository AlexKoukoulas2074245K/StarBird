///------------------------------------------------------------------------------------------------
///  BossAIController.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/03/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BossAIController_h
#define BossAIController_h

///------------------------------------------------------------------------------------------------

#include "../../utils/StringUtils.h"

#include <memory>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class Scene;
class LevelUpdater;
class StateMachine;
class IBossAI;
class b2World;
class BossAIController final
{
public:
    BossAIController(Scene& scene, LevelUpdater& levelUpdater, StateMachine& stateMachine, b2World& box2dWorld);
    ~BossAIController();
    
    void UpdateBossAI(const strutils::StringId& bossName, const float dtMillis);
    
private:
    void RegisterBossAIs();
    
private:
    Scene& mScene;
    LevelUpdater& mLevelUpdater;
    StateMachine& mStateMachine;
    b2World& mBox2dWorld;
    std::unordered_map<strutils::StringId, std::unique_ptr<IBossAI>, strutils::StringIdHasher> mBossAIs;
};

///------------------------------------------------------------------------------------------------

#endif /* BossAIController_h */
