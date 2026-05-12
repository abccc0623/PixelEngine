#pragma once
#include "EngineManager.h"
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyID.h>       
#include <Jolt/Physics/Body/MotionType.h>
#include <unordered_map>
#include <vector>
#include "PhysStruct.h"
#include "Collider2D.h"
namespace JPH
{
	class PhysicsSystem;
	class TempAllocatorImpl;
	class JobSystemThreadPool;
	class BodyInterface;
	class Shape;
	class BodyCreationSettings;
}
namespace ECS::Collider2D
{
	struct Collider2DData;
}

struct CollisionEvent;
class PhysListener;
class ColliderFactory;
class Transform;
class EventManager;
class PhysManager : public EngineManager
{
public:
	PhysManager();
	~PhysManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void SetVelocity(JPH::BodyID id, float x, float y, float z);
	void SetVelocityX(JPH::BodyID id, float x);
	void SetVelocityY(JPH::BodyID id, float y);
	void SetVelocityZ(JPH::BodyID id, float z);
	void SetPosition(JPH::BodyID id, float x, float y, float z, bool active);
	void SetRotation(JPH::BodyID id, float x, float y, float z, bool active);
	void SetActive(JPH::BodyID id, bool active);
	bool GetActive(JPH::BodyID id);

	void AddImpulse(JPH::BodyID id, float x, float y, float z);
	void AddForce(JPH::BodyID id, float x, float y, float z);

	JPH::ShapeRefC CreateCollider(ECS::Collider2D::Collider2DData* collider);
	JPH::BodyID CreateRigidbody(PhysRigidbody& rigidbody, JPH::ShapeRefC shapeRef, void* pOwner);

	void DebugDraw(JPH::BodyID id);
	void SyncPhysics(JPH::BodyID id);
private:
	ColliderFactory* colliderFactory = nullptr;
	PhysListener* eventListener = nullptr;
	EventManager* event;

	JPH::PhysicsSystem* physicsSystem = nullptr;
	JPH::TempAllocatorImpl* tempAllocator = nullptr;
	JPH::JobSystemThreadPool* jobSystem = nullptr;
	JPH::BodyInterface* mBodyInterface = nullptr;

	void* mBpInterface = nullptr;
	void* mObjVsBpFilter = nullptr;
	void* mObjVsObjFilter = nullptr;

	std::unordered_map<std::string, JPH::ShapeRefC> colliderMap;
	std::vector<CollisionEvent> mFrameCollisionEvents;
};

