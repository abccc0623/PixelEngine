#pragma once
#include "EngineManager.h"
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyID.h>       
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
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
namespace ECS
{
	namespace Collider2D
	{
		struct Collider2DData;
	}
	namespace Rigidbody2D
	{
		struct Rigidbody2DData;
	}
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

	Pixel::Vector3 GetVelocity(JPH::BodyID id);
	void SetVelocity(JPH::BodyID id, float x, float y, float z);
	void SetPosition(JPH::BodyID id, float x, float y, float z, bool active);
	void SetRotation(JPH::BodyID id, float x, float y, float z, bool active);
	void SetActive(JPH::BodyID id, bool active);
	bool GetActive(JPH::BodyID id);


	void AddImpulse(JPH::BodyID id, float x, float y, float z);
	void AddForce(JPH::BodyID id, float x, float y, float z);

	JPH::ShapeRefC CreateCollider(ECS::Collider2D::Collider2DData* collider);
	JPH::BodyID CreateRigidbody(ECS::Rigidbody2D::Rigidbody2DData* rigidbody, JPH::ShapeRefC shapeRef, unsigned int pOwner);

	void DebugDraw(JPH::BodyID id);
	void SyncPhysics(JPH::BodyID id);
private:
	JPH::ObjectLayer FindLayer(std::string key);

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
	std::unordered_map<std::string, int> layerList;
};

