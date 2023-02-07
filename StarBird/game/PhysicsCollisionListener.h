///------------------------------------------------------------------------------------------------
///  PhysicsCollisionListener.h                                                                                          
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/02/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef PhysicsCollisionListener_h
#define PhysicsCollisionListener_h

///------------------------------------------------------------------------------------------------

#include "PhysicsConstants.h"

#include <Box2D/Box2D.h>
#include <map>

///------------------------------------------------------------------------------------------------

class UnorderedCollisionCategoryPair
{
public:
    friend class PhysicsCollisionListener;
    
    UnorderedCollisionCategoryPair(uint16 firstCollisionCategory, uint16 secondCollisionCategory)
    : mFirstCollisionCategory(firstCollisionCategory)
    , mSecondCollisionCategory(secondCollisionCategory) {}
    
    struct Comparator
    {
        bool operator()(const UnorderedCollisionCategoryPair& lhs, const UnorderedCollisionCategoryPair& rhs) const
        {
            if (lhs.mFirstCollisionCategory != rhs.mFirstCollisionCategory)
            {
                return lhs.mFirstCollisionCategory < rhs.mFirstCollisionCategory;
            }
            else
            {
                return lhs.mSecondCollisionCategory < rhs.mSecondCollisionCategory;
            }
        }
    };
private:
    uint16 mFirstCollisionCategory;
    uint16 mSecondCollisionCategory;
};

///------------------------------------------------------------------------------------------------

class PhysicsCollisionListener : public b2ContactListener
{
public:
    using CollisionCallback = std::function<void(b2Body* firstBody, b2Body* secondBody)>;
    
    PhysicsCollisionListener();
    
    void RegisterCollisionCallback(UnorderedCollisionCategoryPair&& collisionCategoryPair, CollisionCallback callback);
    
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    
private:
    std::map<UnorderedCollisionCategoryPair, CollisionCallback, UnorderedCollisionCategoryPair::Comparator> mCollisionCallbacks;
    UnorderedCollisionCategoryPair mCollisionPair;
};


///------------------------------------------------------------------------------------------------

#endif /* PhysicsCollisionListener_h */
