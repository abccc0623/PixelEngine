#include "pch.h"
#include "PhysListener.h"
#include "EventManager.h"
#include "PixelEngine.h"
#include "Type/GlobalEnum.h"

extern PixelEngine* Engine;
PhysListener::PhysListener()
{
    event = Engine->GetFactory<EventManager>();
}

PhysListener::~PhysListener()
{

}

void PhysListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
    CollisionEvent ev;
    ev.body1 = inBody1.GetID();
    ev.body2 = inBody2.GetID();
    
    ev.contactPosition = inManifold.GetWorldSpaceContactPointOn1(0);
    ev.contactNormal = inManifold.mWorldSpaceNormal;
    ev.isEnter = true;
    EnqueueEvent(ev);
}

void PhysListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
    CollisionEvent ev;
    ev.body1 = inSubShapePair.GetBody1ID();
    ev.body2 = inSubShapePair.GetBody2ID();
    ev.isEnter = false;
    EnqueueEvent(ev);
}

void PhysListener::EnqueueEvent(const CollisionEvent& ev)
{
    std::lock_guard<std::mutex> lock(mEventMutex);
    mEventQueue.push_back(ev);
}

void PhysListener::FlushEvents(std::vector<CollisionEvent>& outEvents)
{
    std::lock_guard<std::mutex> lock(mEventMutex);
    outEvents.swap(mEventQueue);
}
