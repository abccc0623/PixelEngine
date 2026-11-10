#include "pch.h"
#include "Rigidbody2D.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "PhysManager.h"
#include "ColliderFactory.h"
#include "Physics2DData.h"
#include "PixelMetaAPI.h"

extern PixelEngine* Engine;
namespace
{
	Physics2DData* FindPhysicsData(ECS::Registry* registry, unsigned int id, const char* operation)
	{
		auto data = registry->Get<Physics2DData>(id);
		if (data == nullptr)
		{
			PixelLog::Error(std::string("[Rigidbody2D][") + operation +
				"] Missing Physics2D component. Entity ID: " + std::to_string(id));
		}
		return data;
	}

	bool SetLockedAxes(unsigned int id, bool x, bool y, bool z, bool rotation)
	{
		const char* operation = rotation ? "SetLockRotation" : "SetLockPosition";
		auto data = FindPhysicsData(GetRegistry(), id, operation);
		if (data == nullptr) return false;
		using DOF = JPH::EAllowedDOFs;
		const DOF axisX = rotation ? DOF::RotationX : DOF::TranslationX;
		const DOF axisY = rotation ? DOF::RotationY : DOF::TranslationY;
		const DOF axisZ = rotation ? DOF::RotationZ : DOF::TranslationZ;
		auto allowed = data->allowedDOFs & ~(axisX | axisY | axisZ);
		if (!x) allowed |= axisX;
		if (!y) allowed |= axisY;
		if (!z) allowed |= axisZ;
		if (!data->bodyID.IsInvalid())
		{
			auto physics = Engine->GetFactory<PhysicsFunction>();
			if (!physics->SetAllowedDOFs(data->bodyID, allowed, data->Motiontype))
			{
				PixelLog::Error(std::string("[Rigidbody2D][") + operation + "] Cannot update body constraints. Entity ID: " + std::to_string(id));
				return false;
			}
		}
		data->allowedDOFs = allowed;
		return true;
	}
}

bool Rigidbody2D_SetLockPosition(unsigned int id, bool x, bool y, bool z)
{
	return SetLockedAxes(id, x, y, z, false);
}

bool Rigidbody2D_SetLockRotation(unsigned int id, bool x, bool y, bool z)
{
	return SetLockedAxes(id, x, y, z, true);
}

Rigidbody2DData* Rigidbody2D_Add(unsigned int id)
{
	auto registry = GetRegistry();
	if (!registry->TryAddComponents<Rigidbody2DData, Physics2DData>(id)) return nullptr;
	auto data1 = registry->Get<Rigidbody2DData>(id);
	auto data2 = registry->Get<Physics2DData>(id);
	data1->thisID = id;
	data2->thisID = id;
	return data1;
}

Rigidbody2DData* Rigidbody2D_Get(unsigned int id)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	return data;
}

bool Rigidbody2D_Has(unsigned int id)
{
	auto registry = GetRegistry();
	return registry->Has<Rigidbody2DData>(id);
}

void Rigidbody2D_CreateBoxCollider(unsigned int id, PVector2 size, PVector2 center)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "CreateBoxCollider");
	if (data1 == nullptr) return;
	auto collider = ColliderFactory::CreateBox2D(&size, &center);
	if (collider == nullptr) return;
	data1->colliderRefC = collider;
	data1->colliderType = Physics2DData::BOX;
}

void Rigidbody2D_CreateCircleCollider(unsigned int id, float radius, PVector2 center)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "CreateCircleCollider");
	if (data1 == nullptr) return;
	auto collider = ColliderFactory::CreateCircle2D(radius, &center);
	if (collider == nullptr) return;
	data1->colliderRefC = collider;
	data1->colliderType = Physics2DData::CIRCLE;
}

void Rigidbody2D_SetMotionType(unsigned int id, MotionType motionType)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetMotionType");
	if (data1 == nullptr) return;

	switch (motionType)
	{
	case MotionType::Kinematic:
		data1->Motiontype = JPH::EMotionType::Kinematic;
		break;
	case MotionType::Dynamic:
		data1->Motiontype = JPH::EMotionType::Dynamic;
		break;
	case MotionType::Static:
		data1->Motiontype = JPH::EMotionType::Static;
		break;
	}

	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetAllowedDOFs(data1->bodyID, data1->allowedDOFs, data1->Motiontype);
	}
}

void Rigidbody2D_SetPosition(unsigned int id, PVector3 position)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetPosition");
	if (data1 == nullptr) return;
	auto transform = registry->Get<TransformData>(id);
	if (transform == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][SetPosition] Missing Transform component. Entity ID: " + std::to_string(id));
		return;
	}
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetPosition(data1->bodyID, position.x, position.y, position.z);
	}
	transform->position.x = position.x;
	transform->position.y = position.y;
	transform->position.z = position.z;
}

void Rigidbody2D_SetRotation(unsigned int id, PVector3 rotation)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetRotation");
	if (data1 == nullptr) return;
	auto transform = registry->Get<TransformData>(id);
	if (transform == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][SetRotation] Missing Transform component. Entity ID: " + std::to_string(id));
		return;
	}
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetRotation(data1->bodyID, rotation.x, rotation.y, rotation.z);
	}
	transform->rotation.x = rotation.x;
	transform->rotation.y = rotation.y;
	transform->rotation.z = rotation.z;
}

void Rigidbody2D_SetVelocity(unsigned int id, PVector3 velocity)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetVelocity");
	if (data1 == nullptr) return;
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetVelocity(data1->bodyID, velocity.x, velocity.y, velocity.z);
	}
	else
	{
		if (data1->Create == true)
		{
			PixelLog::Error("[Rigidbody2D][SetVelocity] Invalid physics body. Entity ID: " + std::to_string(id));
		}
	}
}

void Rigidbody2D_SetGravity(unsigned int id, float gravity)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetGravity");
	if (data1 == nullptr) return;
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetGravity(data1->bodyID, gravity);
	}
	else
	{
		data1->gravity = gravity;
	}
}

void Rigidbody2D_SetRestitution(unsigned int id, float restitution)
{

}

void Rigidbody2D_SetFriction(unsigned int id, float restitution)
{

}

void Rigidbody2D_SetLinearDamping(unsigned int id, float linearDamping)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetLinearDamping");
	if (data1 == nullptr) return;

	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetLinearDamping(data1->bodyID, linearDamping);
	}
	else
	{
		data1->linearDamping = linearDamping;
	}
}

void Rigidbody2D_SetSensor(unsigned int id, bool sensor)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetSensor");
	if (data1 == nullptr) return;
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetSensor(data1->bodyID, sensor);
	}
	data1->sensor = sensor;
}

void Rigidbody2D_SetActive(unsigned int id, bool active)
{
	auto registry = GetRegistry();
	auto data1 = FindPhysicsData(registry, id, "SetActive");
	if (data1 == nullptr) return;
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetActive(data1->bodyID, active);
	}
	data1->active = active;
}

void Rigidbody2D_SetLayer(unsigned int id, const char* layerName)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
	if (data1 == nullptr || layerName == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][SetLayer] Invalid Component Or LayerName");
		return;
	}
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetLayer(data1->bodyID, layerName);
	}
	data1->layer = layerName;
}

const char* Rigidbody2D_GetLayer(unsigned int id)
{
	auto data = GetRegistry()->Get<Physics2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][GetLayer] Not Find Component");
		return "";
	}
	return data->layer.c_str();
}
