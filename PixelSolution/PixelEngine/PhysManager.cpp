#include "pch.h"
#include "PhysManager.h"
#include "PixelEngineAPI.h"
#include "Type/GlobalEnum.h"
#include "PixelGraphicsAPI.h"
#include "PhysListener.h"
#include "EventManager.h"
#include "PixelEngine.h"
#include "EntityObject.h"
#include "Rigidbody2D.h"
#include "Registry.h"
#include "Transform.h"
#include "PixelMath.h"
#include "PixelEngineAPI.h"
#include "Physics2DData.h"
#include "ColliderFactory.h"
#include "Asset.h"
#include <Jolt/Physics/Body/BodyLock.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "PixelObjectLayerPairFilter.h"
#include "PixelBroadPhaseLayerFilter.h"

using namespace JPH;
JPH::BodyInterface* PhysManager::mBodyInterface = nullptr;
const JPH::BodyLockInterfaceLocking* PhysManager::mBodyLockInterface = nullptr;

PhysManager::PhysManager()
{
	layerList.insert({ "Default", 0 });
	layerList.insert({ "Player",  1 });
	layerList.insert({ "Ground",  2 });
}

PhysManager::~PhysManager()
{
	if (physicsSystem)
	{
		physicsSystem->SetContactListener(nullptr);
	}

	delete eventListener;

	delete physicsSystem;
	physicsSystem = nullptr;

	delete mObjVsObjFilter;
	delete mObjVsBpFilter;
	delete mBpInterface;

	delete jobSystem;
	delete tempAllocator;

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}
void MyTrace(const char* inFMT, ...) {
	char buffer[1024];
	va_list list;
	va_start(list, inFMT);
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	std::string log(buffer);
	PixelLog::Info(log);
}

extern PixelEngine* Engine;
void PhysManager::Initialize()
{
	std::string layerPath = Engine->GetEngineRootFolderPath() + "/Engine/LayerMatrix.json";
	Asset_Import(layerPath.c_str());

	// 2. Jolt에게 "할 말 있으면 여기다 말해"라고 등록
	JPH::Trace = MyTrace;

	RegisterDefaultAllocator();
	Factory::sInstance = new Factory();
	RegisterTypes();

	// 2. 실행 환경 메모리/스레드 설정
	tempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);
	jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);


	// 3. 필터 인스턴스 생성 (나중에 해제 필요)
	mBpInterface = new PixelBroadPhaseLayerInterface();
	mObjVsBpFilter = new PixelBroadPhaseLayerFilter();
	mObjVsObjFilter = new PixelObjectLayerPairFilter();

	// 4. 물리 시스템 생성
	physicsSystem = new PhysicsSystem();
	physicsSystem->Init(
		10240, 0, 1024, 1024,
		*static_cast<PixelBroadPhaseLayerInterface*>(mBpInterface),
		*static_cast<PixelBroadPhaseLayerFilter*>(mObjVsBpFilter),
		*static_cast<PixelObjectLayerPairFilter*>(mObjVsObjFilter)
	);

	eventListener = new PhysListener();

	//event = Engine->GetFactory<EventManager>();
	mBodyInterface = &physicsSystem->GetBodyInterface();
	mBodyLockInterface = &physicsSystem->GetBodyLockInterface();
	physicsSystem->SetContactListener(eventListener);
}

void PhysManager::Update()
{
	const float deltaTime = GetDeltaTime();
	if (deltaTime <= 0.0f)
	{
		return;
	}
	physicsSystem->Update(deltaTime, 1, tempAllocator, jobSystem);

	mFrameCollisionEvents.clear();
	eventListener->FlushEvents(mFrameCollisionEvents);

	for (const auto& ev : mFrameCollisionEvents)
	{
		auto entity1 = FindCollisionEntity(ev.body1);
		auto entity2 = FindCollisionEntity(ev.body2);
		if (entity1 == nullptr || entity2 == nullptr) continue;

		const unsigned int target1 = entity1->GetID();
		const unsigned int target2 = entity2->GetID();
		if (ev.isEnter)
		{
			entity1->OnCollisionEnter(target2);
		}
		else
		{
			entity1->OnCollisionExit(target2);
		}

		// Lua may destroy either entity (or their parent) in the first callback.
		// Do not keep body locks or entity pointers across that callback.
		if (FindCollisionEntity(ev.body1) == nullptr) continue;
		entity2 = FindCollisionEntity(ev.body2);
		if (entity2 == nullptr) continue;
		if (ev.isEnter)
		{
			entity2->OnCollisionEnter(target1);
		}
		else
		{
			entity2->OnCollisionExit(target1);
		}
	}
}

ECS::EntityObject* PhysManager::FindCollisionEntity(JPH::BodyID id)
{
	if (id.IsInvalid() || mBodyLockInterface == nullptr) return nullptr;

	unsigned int entityID;
	{
		// Removed-contact events can arrive after DestroyBody. Validate the full
		// BodyID (including its sequence number) before reading any body data.
		JPH::BodyLockRead lock(*mBodyLockInterface, id);
		if (!lock.Succeeded()) return nullptr;
		entityID = static_cast<unsigned int>(lock.GetBody().GetUserData());
	}

	auto registry = GetRegistry();
	if (registry == nullptr) return nullptr;
	auto physics = registry->Get<Physics2DData>(entityID);
	if (physics == nullptr || physics->bodyID != id) return nullptr;
	return FindEntity(entityID);
}

void PhysManager::DestroyRigidbody(Physics2DData* physics)
{
	if (physics == nullptr) return;
	const JPH::BodyID id = physics->bodyID;
	physics->bodyID = JPH::BodyID();
	physics->Create = false;
	if (id.IsInvalid() || mBodyInterface == nullptr || mBodyLockInterface == nullptr) return;

	{
		JPH::BodyLockRead lock(*mBodyLockInterface, id);
		if (!lock.Succeeded()) return;
	}
	if (mBodyInterface->IsAdded(id))
	{
		mBodyInterface->RemoveBody(id);
	}
	mBodyInterface->DestroyBody(id);
}

void PhysManager::ClearCollisionEvents()
{
	mFrameCollisionEvents.clear();
	if (eventListener != nullptr) eventListener->FlushEvents(mFrameCollisionEvents);
	mFrameCollisionEvents.clear();
}

void PhysManager::Clear()
{
	if (mBodyInterface == nullptr) return;

	std::string layerPath = Engine->GetEngineRootFolderPath() + "/Engine/LayerMatrix.json";
	Asset_Import(layerPath.c_str());

	JPH::BodyIDVector allBodies;
	physicsSystem->GetBodies(allBodies);

	JPH::BodyIDVector addedBodies;
	JPH::BodyIDVector removedBodies;
	addedBodies.reserve(allBodies.size());
	removedBodies.reserve(allBodies.size());


	for (const JPH::BodyID& id : allBodies)
	{
		if (mBodyInterface->IsAdded(id))
		{
			addedBodies.push_back(id);
		}
		else
		{
			// 이미 Remove된 상태의 객체 (오브젝트 풀 대기열 등)
			removedBodies.push_back(id);
		}
	}
	if (!addedBodies.empty())
	{
		mBodyInterface->RemoveBodies(addedBodies.data(), (int)addedBodies.size());
		mBodyInterface->DestroyBodies(addedBodies.data(), (int)addedBodies.size());
	}

	if (!removedBodies.empty())
	{
		mBodyInterface->DestroyBodies(removedBodies.data(), (int)removedBodies.size());
	}

	ClearCollisionEvents();
	colliderMap.clear();
}

void PhysManager::Release()
{

}


Pixel::Vector3 PhysManager::GetVelocity(JPH::BodyID id)
{
	auto vec3 = physicsSystem->GetBodyInterface().GetLinearVelocity(id);
	return Pixel::Vector3(vec3.GetX(), vec3.GetY(), vec3.GetZ());
}

void PhysManager::SetVelocity(JPH::BodyID id, float x, float y, float z)
{
	JPH::Vec3 targetVelocity(x, y, 0.0f);
	physicsSystem->GetBodyInterface().SetLinearVelocity(id, targetVelocity);
}

void PhysManager::SetPosition(JPH::BodyID id, float x, float y, float z, bool active)
{
	JPH::Vec3 targetPosition(x, y, 0.0f);
	physicsSystem->GetBodyInterface().SetPosition(id, targetPosition, (active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void PhysManager::SetRotation(JPH::BodyID id, float x, float y, float z, bool active)
{
	constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
	JPH::Vec3 eulerRadians(x * DEG_TO_RAD, y * DEG_TO_RAD, z * DEG_TO_RAD);
	JPH::Quat targetRotation = JPH::Quat::sEulerAngles(eulerRadians);
	physicsSystem->GetBodyInterface().SetRotation(
		id,
		targetRotation,
		(active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate
	);
}

void PhysManager::SetActive(JPH::BodyID id, bool active)
{
	if (id.IsInvalid()) return;

	if (active)
	{
		if (mBodyInterface->IsAdded(id))
		{
			PixelLog::Warn("ActivatePhysics: BodyID {} is already added to the world!");
			return;
		}

		unsigned int userData = mBodyInterface->GetUserData(id);
		auto data = reinterpret_cast<TransformData*>(Transform_Get(userData));


		JPH::RVec3 RPos(data->position.x, data->position.y, 0.0f);
		constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
		JPH::Vec3 eulerRadians(data->rotation.x * DEG_TO_RAD, data->rotation.y * DEG_TO_RAD, data->rotation.z * DEG_TO_RAD);
		JPH::Quat RRot = JPH::Quat::sEulerAngles(eulerRadians);

		mBodyInterface->SetLinearAndAngularVelocity(id, JPH::Vec3::sZero(), JPH::Vec3::sZero());
		mBodyInterface->SetPositionAndRotation(id, RPos, RRot, JPH::EActivation::DontActivate);
		mBodyInterface->AddBody(id, JPH::EActivation::Activate);
	}
	else
	{
		mBodyInterface->RemoveBody(id);
	}
}

bool PhysManager::GetActive(JPH::BodyID id)
{
	return mBodyInterface->IsAdded(id) ? true : false;
}

void PhysManager::AddImpulse(JPH::BodyID id, float x, float y, float z)
{
	physicsSystem->GetBodyInterface().AddImpulse(id, JPH::Vec3(x, y, z));
}
void PhysManager::AddForce(JPH::BodyID id, float x, float y, float z)
{
	physicsSystem->GetBodyInterface().AddForce(id, JPH::Vec3(x, y, z));
}


//JPH::BodyID PhysManager::CreateRigidbody(ECS::Rigidbody2D::Rigidbody2DData* rigidbody, JPH::ShapeRefC shapeRef, unsigned int pOwner)
//{
	//data->
	//JPH::ObjectLayer objectLayer = FindLayer(rigidbody->layer);
	//JPH::EMotionType eMotionType = JPH::EMotionType::Dynamic;
	//JPH::EActivation active = (rigidbody->Active) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
	//
	//switch (rigidbody->type)
	//{
	//case MotionType::Static:
	//	eMotionType = EMotionType::Static;
	//	break;
	//case MotionType::Kinematic:
	//	eMotionType = EMotionType::Kinematic;
	//	break;
	//case MotionType::Dynamic:
	//	eMotionType = EMotionType::Dynamic;
	//	break;
	//}
	//constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
	//
	//auto r = GetRegistry();
	//auto transformData = r->Get<TransformData>(pOwner);
	//
	//JPH::Vec3 eulerRadians(transformData->rotation.x * DEG_TO_RAD, transformData->rotation.y * DEG_TO_RAD, transformData->rotation.z * DEG_TO_RAD);
	//JPH::BodyCreationSettings bodySettings
	//(
	//	shapeRef,
	//	JPH::Vec3(transformData->position.x, transformData->position.y, transformData->position.z),
	//	JPH::Quat::sEulerAngles(eulerRadians), // 초기 회전값
	//	eMotionType,
	//	objectLayer
	//);
	//bodySettings.mGravityFactor = rigidbody->Gravity;
	//bodySettings.mAllowSleeping = rigidbody->AutoSleep;
	//bodySettings.mIsSensor = rigidbody->Sensor;
	//bodySettings.mRestitution = rigidbody->Restitution;
	//bodySettings.mFriction = rigidbody->Friction;
	//bodySettings.mLinearDamping = rigidbody->LinearDamping;
	//
	//bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::All;
	//if (rigidbody->lockPosition[0] == true) bodySettings.mAllowedDOFs &= ~JPH::EAllowedDOFs::TranslationX;
	//if (rigidbody->lockPosition[1] == true) bodySettings.mAllowedDOFs &= ~JPH::EAllowedDOFs::TranslationY;
	//if (rigidbody->lockPosition[2] == true) bodySettings.mAllowedDOFs &= ~JPH::EAllowedDOFs::TranslationZ;
	//if (rigidbody->lockRotation[0] == true) bodySettings.mAllowedDOFs &= ~JPH::EAllowedDOFs::RotationX;
	//if (rigidbody->lockRotation[1] == true) bodySettings.mAllowedDOFs &= ~JPH::EAllowedDOFs::RotationY;
	//if (rigidbody->lockRotation[2] == true) bodySettings.mAllowedDOFs &= ~JPH::EAllowedDOFs::RotationZ;
	//auto id = mBodyInterface->CreateAndAddBody(bodySettings, active);
	//mBodyInterface->SetUserData(id, static_cast<uint64_t>(pOwner));
	//return id;
//return JPH::BodyID(0);
//}

JPH::BodyID PhysManager::CreateRigidbody(Rigidbody2DData* rigidbody, Physics2DData* physics)
{
	if (rigidbody == nullptr || physics == nullptr || physics->colliderRefC == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][CreateBody] Missing component or collider.");
		return JPH::BodyID();
	}
	JPH::ObjectLayer objectLayer = FindLayer(physics->layer);
	JPH::EMotionType eMotionType = JPH::EMotionType::Dynamic;
	JPH::EActivation active = JPH::EActivation::Activate;
	eMotionType = physics->Motiontype;

	constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

	auto r = GetRegistry();
	auto transformData = r->Get<TransformData>(rigidbody->thisID);
	if (transformData == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][CreateBody] Missing Transform component. Entity ID: " + std::to_string(rigidbody->thisID));
		return JPH::BodyID();
	}

	JPH::Vec3 eulerRadians(transformData->rotation.x * DEG_TO_RAD, transformData->rotation.y * DEG_TO_RAD, transformData->rotation.z * DEG_TO_RAD);
	JPH::BodyCreationSettings bodySettings
	(
		physics->colliderRefC,
		JPH::Vec3(transformData->position.x, transformData->position.y, 0.0f),
		JPH::Quat::sEulerAngles(eulerRadians), // 초기 회전값
		eMotionType,
		objectLayer
	);

	bodySettings.mIsSensor = physics->sensor;
	bodySettings.mLinearDamping = physics->linearDamping;
	bodySettings.mGravityFactor = physics->gravity;
	// Retain motion properties so an all-locked (static) body can be unlocked later.
	bodySettings.mAllowDynamicOrKinematic = true;
	bodySettings.mAllowedDOFs = physics->allowedDOFs;
	if (physics->allowedDOFs == JPH::EAllowedDOFs::None)
	{
		bodySettings.mMotionType = JPH::EMotionType::Static;
		bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::All;
	}

	if (physics->active == true)
	{
		auto id = mBodyInterface->CreateAndAddBody(bodySettings, active);
		if (id.IsInvalid())
		{
			PixelLog::Error("[Rigidbody2D][CreateBody] Failed to allocate physics body. Entity ID: " + std::to_string(rigidbody->thisID));
			return id;
		}
		mBodyInterface->SetUserData(id, static_cast<uint64_t>(rigidbody->thisID));
		return id;
	}
	else
	{
		auto body = mBodyInterface->CreateBody(bodySettings);
		if (body == nullptr)
		{
			PixelLog::Error("[Rigidbody2D][CreateBody] Failed to allocate physics body. Entity ID: " + std::to_string(rigidbody->thisID));
			return JPH::BodyID();
		}
		auto id = body->GetID();
		mBodyInterface->SetUserData(id, static_cast<uint64_t>(rigidbody->thisID));
		return id;
	}
}

namespace
{
	void DrawColliderOutline(const JPH::Shape* actualShape, JPH::RVec3 pos, JPH::Quat rot)
	{
		if (actualShape == nullptr) return;
		while (actualShape->GetSubType() == JPH::EShapeSubType::RotatedTranslated)
		{
			auto shape = static_cast<const JPH::RotatedTranslatedShape*>(actualShape);
			pos += rot * shape->GetPosition();
			rot = rot * shape->GetRotation();
			actualShape = shape->GetInnerShape();
		}
		if (actualShape->GetSubType() == JPH::EShapeSubType::Box)
		{
			const auto extent = static_cast<const JPH::BoxShape*>(actualShape)->GetHalfExtent();
			const JPH::Vec3 corners[4] = {
				JPH::Vec3(-extent.GetX(), -extent.GetY(), 0),
				JPH::Vec3(extent.GetX(), -extent.GetY(), 0),
				JPH::Vec3(extent.GetX(), extent.GetY(), 0),
				JPH::Vec3(-extent.GetX(), extent.GetY(), 0)
			};
			for (int i = 0; i < 4; ++i)
			{
				const JPH::RVec3 start = pos + rot * corners[i];
				const JPH::RVec3 end = pos + rot * corners[(i + 1) % 4];
				DrawLine(start.GetX(), start.GetY(), start.GetZ(), end.GetX(), end.GetY(), end.GetZ(), 1.0f, 0.0f, 0.0f);
			}
		}
		else if (actualShape->GetSubType() == JPH::EShapeSubType::Sphere)
		{
			const float radius = static_cast<const JPH::SphereShape*>(actualShape)->GetRadius();
			DrawCircle2D(pos.GetX(), pos.GetY(), pos.GetZ(), radius, 0.0f, 1.0f, 0.0f);
		}
	}
}

void PhysManager::DebugDraw(JPH::BodyID id)
{
	if (Engine == nullptr || !Engine->IsTool || id.IsInvalid()) return;
	if (!mBodyInterface->IsAdded(id)) return;
	const auto shape = mBodyInterface->GetShape(id);
	DrawColliderOutline(shape.GetPtr(), mBodyInterface->GetPosition(id), mBodyInterface->GetRotation(id));
}

void PhysManager::DebugDrawPreview(Rigidbody2DData* rigidbody, Physics2DData* physics)
{
	if (Engine == nullptr || !Engine->IsTool || physics == nullptr || !physics->active) return;
	const auto transform = GetRegistry()->Get<TransformData>(rigidbody->thisID);
	if (transform == nullptr) return;
	auto shape = physics->colliderRefC;
	if (shape == nullptr)
	{
		PVector2 size{ 1,1 }, center{ 0,0 };
		shape = ColliderFactory::CreateBox2D(&size, &center);
	}
	constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
	const JPH::Vec3 angles(transform->rotation.x * DEG_TO_RAD, transform->rotation.y * DEG_TO_RAD, transform->rotation.z * DEG_TO_RAD);
	// Use the same 2D origin and shape as body creation, without advancing simulation.
	DrawColliderOutline(shape.GetPtr(), JPH::RVec3(transform->position.x, transform->position.y, 0.0f), JPH::Quat::sEulerAngles(angles));
}

void PhysManager::SyncPhysics(JPH::BodyID id)
{
	if (id.IsInvalid()) return;
	unsigned int userData = mBodyInterface->GetUserData(id);
	if (mBodyInterface->IsAdded(id) == false) return;
	auto EntityID = static_cast<unsigned int>(userData);
	JPH::RVec3 pos = mBodyInterface->GetPosition(id);
	JPH::Quat rot = mBodyInterface->GetRotation(id);
	auto r = GetRegistry();
	auto worldData = r->Get<WorldData>(EntityID);
	auto transformData = r->Get<TransformData>(EntityID);
	if (transformData == nullptr) return;
	transformData->position.x = pos.GetX();
	transformData->position.y = pos.GetY();
	constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
	const auto angles = rot.GetEulerAngles();
	transformData->rotation.x = angles.GetX() * RAD_TO_DEG;
	transformData->rotation.y = angles.GetY() * RAD_TO_DEG;
	transformData->rotation.z = angles.GetZ() * RAD_TO_DEG;
	DebugDraw(id);
}

JPH::ObjectLayer PhysManager::FindLayer(std::string key)
{
	int size = Layers::layerNames.size();
	for (int i = 0; i < size; i++)
	{
		if (Layers::layerNames[i] == key)
		{
			return JPH::ObjectLayer(i);
		}
	}

	PixelLog::Error("Not Find Layer Name :" + key);
	return JPH::ObjectLayer(0);
}


