///------------------------------------------------------------------------------------------------
///  PhysicsCollisionListener.cpp                                                                                        
///  StarBird                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/02/2023                                                       
///------------------------------------------------------------------------------------------------

#include "PhysicsCollisionListener.h"
#include "Logging.h"

///------------------------------------------------------------------------------------------------

PhysicsCollisionListener::PhysicsCollisionListener()
    : mCollisionPair(0, 0)
{
}

///------------------------------------------------------------------------------------------------

void PhysicsCollisionListener::RegisterCollisionCallback(UnorderedCollisionCategoryPair &&collisionCategoryPair, CollisionCallback callback)
{
    mCollisionCallbacks[collisionCategoryPair] = callback;
}

///------------------------------------------------------------------------------------------------

void PhysicsCollisionListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    (void)oldManifold;
    
    mCollisionPair.mFirstCollisionCategory  = contact->GetFixtureA()->GetFilterData().categoryBits;
    mCollisionPair.mSecondCollisionCategory = contact->GetFixtureB()->GetFilterData().categoryBits;

    auto findIter = mCollisionCallbacks.find(mCollisionPair);
    if (findIter != mCollisionCallbacks.end())
    {
        findIter->second(contact->GetFixtureA()->GetBody(), contact->GetFixtureB()->GetBody());
        
        // If the user mistakenly adds a reverse pair collision response this avoids resolving the collision twice
        return;
    }
    
    auto temp = mCollisionPair.mFirstCollisionCategory;
    mCollisionPair.mFirstCollisionCategory = mCollisionPair.mSecondCollisionCategory;
    mCollisionPair.mSecondCollisionCategory = temp;
    
    findIter = mCollisionCallbacks.find(mCollisionPair);
    if (findIter != mCollisionCallbacks.end())
    {
        // If the user mistakenly adds a reverse pair collision response this avoids resolving the collision twice
        findIter->second(contact->GetFixtureB()->GetBody(), contact->GetFixtureA()->GetBody());
        return;
    }
}

///------------------------------------------------------------------------------------------------
