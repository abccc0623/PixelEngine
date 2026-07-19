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
#include "Asset.h"
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
	physicsSystem->Update(GetDeltaTime(), 1, tempAllocator, jobSystem);

	mFrameCollisionEvents.clear();
	eventListener->FlushEvents(mFrameCollisionEvents);

	for (const auto& ev : mFrameCollisionEvents)
	{
		EventMessage eventMessage;
		eventMessage.Collision.targetIn = ev.isEnter;

		auto target1 = static_cast<uint64_t>(mBodyInterface->GetUserData(ev.body1));
		auto target2 = static_cast<uint64_t>(mBodyInterface->GetUserData(ev.body2));

		ECS::EntityObject* EntityTarget1 = FindEntity(target1);
		ECS::EntityObject* EntityTarget2 = FindEntity(target2);

		if (ev.isEnter == true)
		{
			EntityTarget1->OnCollisionEnter(target2);
			EntityTarget2->OnCollisionEnter(target1);
		}
		else
		{
			EntityTarget1->OnCollisionExit(target2);
			EntityTarget2->OnCollisionExit(target1);
		}
	}
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
	JPH::Vec3 targetVelocity(x, y, z);
	physicsSystem->GetBodyInterface().SetLinearVelocity(id, targetVelocity);
}

void PhysManager::SetPosition(JPH::BodyID id, float x, float y, float z, bool active)
{
	JPH::Vec3 targetPosition(x, y, z);
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


		JPH::RVec3 RPos(data->position.x, data->position.y, data->position.z);
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
	JPH::ObjectLayer objectLayer = FindLayer(physics->layer);
	JPH::EMotionType eMotionType = JPH::EMotionType::Dynamic;
	JPH::EActivation active = JPH::EActivation::Activate;
	eMotionType = physics->Motiontype;

	constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

	auto r = GetRegistry();
	auto transformData = r->Get<TransformData>(rigidbody->thisID);

	JPH::Vec3 eulerRadians(transformData->rotation.x * DEG_TO_RAD, transformData->rotation.y * DEG_TO_RAD, transformData->rotation.z * DEG_TO_RAD);
	JPH::BodyCreationSettings bodySettings
	(
		physics->colliderRefC,
		JPH::Vec3(transformData->position.x, transformData->position.y, transformData->position.z),
		JPH::Quat::sEulerAngles(eulerRadians), // 초기 회전값
		eMotionType,
		objectLayer
	);

	bodySettings.mIsSensor = physics->sensor;
	bodySettings.mLinearDamping = physics->linearDamping;

	if (physics->active == true)
	{
		auto id = mBodyInterface->CreateAndAddBody(bodySettings, active);
		mBodyInterface->SetUserData(id, static_cast<uint64_t>(rigidbody->thisID));
		return id;
	}
	else
	{
		auto body = mBodyInterface->CreateBody(bodySettings);
		auto id = body->GetID();
		mBodyInterface->SetUserData(id, static_cast<uint64_t>(rigidbody->thisID));
		return id;
	}
}

void PhysManager::DebugDraw(JPH::BodyID id)
{
	JPH::ShapeRefC rootShape = mBodyInterface->GetShape(id);
	if (rootShape == nullptr) return;

	if (mBodyInterface->IsAdded(id) == false) return;

	JPH::RVec3 pos = mBodyInterface->GetPosition(id);
	JPH::Quat rot = mBodyInterface->GetRotation(id);

	// 1. 데코레이터(껍데기) 해제 및 로컬 오프셋 추출
	JPH::Vec3 localOffset = JPH::Vec3::sZero();
	const JPH::Shape* actualShape = rootShape.GetPtr();

	if (actualShape->GetSubType() == JPH::EShapeSubType::RotatedTranslated)
	{
		auto rtShape = static_cast<const JPH::RotatedTranslatedShape*>(actualShape);
		localOffset = rtShape->GetPosition();

		actualShape = rtShape->GetInnerShape();
	}

	// 2. 실제 Shape 타입에 따른 렌더링 분기
	switch (actualShape->GetSubType())
	{
	case JPH::EShapeSubType::Box:
	{
		auto boxShape = static_cast<const JPH::BoxShape*>(actualShape);
		JPH::Vec3 extent = boxShape->GetHalfExtent(); // AABox 대신 실제 박스의 절반 크기를 가져옴

		JPH::Vec3 p[4] = {
			JPH::Vec3(localOffset.GetX() - extent.GetX(), localOffset.GetY() - extent.GetY(), 0),
			JPH::Vec3(localOffset.GetX() + extent.GetX(), localOffset.GetY() - extent.GetY(), 0),
			JPH::Vec3(localOffset.GetX() + extent.GetX(), localOffset.GetY() + extent.GetY(), 0),
			JPH::Vec3(localOffset.GetX() - extent.GetX(), localOffset.GetY() + extent.GetY(), 0)
		};
		JPH::Vec3 w[4];
		for (int i = 0; i < 4; ++i)
		{
			w[i] = pos + rot * p[i];
		}
		DrawLine(w[0].GetX(), w[0].GetY(), 0, w[1].GetX(), w[1].GetY(), 0, 1.0f, 0.0f, 0.0f);
		DrawLine(w[1].GetX(), w[1].GetY(), 0, w[2].GetX(), w[2].GetY(), 0, 1.0f, 0.0f, 0.0f);
		DrawLine(w[2].GetX(), w[2].GetY(), 0, w[3].GetX(), w[3].GetY(), 0, 1.0f, 0.0f, 0.0f);
		DrawLine(w[3].GetX(), w[3].GetY(), 0, w[0].GetX(), w[0].GetY(), 0, 1.0f, 0.0f, 0.0f);
		break;
	}
	case JPH::EShapeSubType::Sphere:
	{
		auto sphereShape = static_cast<const JPH::SphereShape*>(actualShape);
		float radius = sphereShape->GetRadius();
		JPH::Vec3 worldCenter = pos + rot * localOffset;
		DrawCircle2D(worldCenter.GetX(), worldCenter.GetY(), 0, radius, 0.0f, 1.0f, 0.0f);
		break;
	}
	default:
	{
		break;
	}
	}
}

void PhysManager::SyncPhysics(JPH::BodyID id)
{
	unsigned int userData = mBodyInterface->GetUserData(id);
	if (mBodyInterface->IsAdded(id) == false) return;
	auto EntityID = static_cast<unsigned int>(userData);
	JPH::RVec3 pos = mBodyInterface->GetPosition(id);
	JPH::Quat rot = mBodyInterface->GetRotation(id);
	auto r = GetRegistry();
	auto worldData = r->Get<WorldData>(EntityID);
	auto transformData = r->Get<TransformData>(EntityID);
	transformData->position.x = pos.GetX();
	transformData->position.y = pos.GetY();
	transformData->position.z = pos.GetZ();
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


