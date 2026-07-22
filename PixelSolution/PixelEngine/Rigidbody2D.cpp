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
/*

void* ECS::Rigidbody2D::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	return data;
}
void* ECS::Rigidbody2D::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	Rigidbody2DData* data = registry->Get<Rigidbody2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][GetComponent] Not Find Component");
	}
	return data;
}
bool ECS::Rigidbody2D::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	Rigidbody2DData* data = registry->Get<Rigidbody2DData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ECS::Rigidbody2D::SetLayer(unsigned int id, const char* name)
{
	auto registry = GetRegistry();
	if (registry->Has<Rigidbody2DData>(id))
	{
		auto data = registry->Get<Rigidbody2DData>(id);
		data->layer = name;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetLayer] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetPosition(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	if (registry->Has<Rigidbody2DData>(id) == false)
	{
		registry->Add<Rigidbody2DData>(id);
	}
	if (registry->Has<TransformData>(id))
	{
		registry->Add<TransformData>(id);
	}

	auto transformData = registry->Get<TransformData>(id);
	auto rigidbodyData = registry->Get<Rigidbody2DData>(id);
	if (transformData != nullptr)
	{
		transformData->position.x = x;
		transformData->position.y = y;
		transformData->position.z = z;
	}

	if (rigidbodyData != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->SetPosition(JPH::BodyID(rigidbodyData->bodyID), x, y, z, true);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	if (registry->Has<Rigidbody2DData>(id) == false)
	{
		registry->Add<Rigidbody2DData>(id);
	}
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->SetRotation(JPH::BodyID(data->bodyID), x, y, z, true);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetRotation] Not Find Component");
	}
}


void ECS::Rigidbody2D::LockPosition(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		if (data->IsCreate)
		{

		}
		else
		{

		}
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][LockPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::LockRotation(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		if (data->IsCreate)
		{

		}
		else
		{

		}
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][LockRotation] Not Find Component");
	}
}
*/

Rigidbody2DData* Rigidbody2D_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	registry->Add<Physics2DData>(id);
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
	auto data1 = registry->Get<Physics2DData>(id);
	data1->colliderRefC = ColliderFactory::CreateBox2D(&size, &center);
	data1->colliderType = Physics2DData::BOX;
}

void Rigidbody2D_CreateCircleCollider(unsigned int id, float radius, PVector2 center)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
	data1->colliderRefC = ColliderFactory::CreateCircle2D(radius, &center);
	data1->colliderType = Physics2DData::CIRCLE;
}

void Rigidbody2D_SetMotionType(unsigned int id, MotionType motionType)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);

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

	if (data1->bodyID.IsInvalid() == true)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetMotionType(data1->bodyID, data1->Motiontype);
	}
}

void Rigidbody2D_SetPosition(unsigned int id, PVector3 position)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetPosition(data1->bodyID, position.x, position.y, position.z);
	}
	auto transform = registry->Get<TransformData>(id);
	transform->position.x = position.x;
	transform->position.y = position.y;
	transform->position.z = position.z;
}

void Rigidbody2D_SetRotation(unsigned int id, PVector3 rotation)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetRotation(data1->bodyID, rotation.x, rotation.y, rotation.z);
	}
	auto transform = registry->Get<TransformData>(id);
	transform->rotation.x = rotation.x;
	transform->rotation.y = rotation.y;
	transform->rotation.z = rotation.z;
}

void Rigidbody2D_SetVelocity(unsigned int id, PVector3 velocity)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetVelocity(data1->bodyID, velocity.x, velocity.y, velocity.z);
	}
	else
	{
		if (data1->Create == true)
		{
			PixelLog::Error("[Rigidbody2D][SetVelocity] Not Find Collider");
		}
	}
}

void Rigidbody2D_SetGravity(unsigned int id, float gravity)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
	if (data1->bodyID.IsInvalid())
	{

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

}

void Rigidbody2D_SetSensor(unsigned int id, bool sensor)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Physics2DData>(id);
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
	auto data1 = registry->Get<Physics2DData>(id);
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
	if (data1->bodyID.IsInvalid() == false)
	{
		auto phys = Engine->GetFactory<PhysicsFunction>();
		phys->SetLayer(data1->bodyID, layerName);
	}
	data1->layer = layerName;
}

