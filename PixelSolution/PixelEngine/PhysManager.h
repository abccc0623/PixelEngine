#pragma once
#include "EngineManager.h"
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyID.h>       
#include <Jolt/Physics/Body/MotionType.h> 
#include <vector>

namespace JPH 
{
	class PhysicsSystem;
	class TempAllocatorImpl;
	class JobSystemThreadPool;
	class BodyInterface;
	class Shape;
	class BodyCreationSettings;
}

class Transform;
class PhysManager : public EngineManager
{
public:
	PhysManager();
	~PhysManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void CollisionUpdate();

	JPH::ShapeRefC CreateBoxCollider2D(float x,float y);
	JPH::BodyID CreateBody2D(JPH::BodyCreationSettings* setting,bool active, void* pOwner);
	void SyncPhysics(JPH::BodyID id);
private:
	JPH::PhysicsSystem* physicsSystem		= nullptr;
	JPH::TempAllocatorImpl* tempAllocator	= nullptr;
	JPH::JobSystemThreadPool* jobSystem		= nullptr;
	JPH::BodyInterface* mBodyInterface		= nullptr;

	void* mBpInterface = nullptr;
	void* mObjVsBpFilter = nullptr;
	void* mObjVsObjFilter = nullptr;
};

