#include "pch.h"
#include "Rigidbody2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "PhysManager.h"

extern PixelEngine* Engine;
void* ECS::Rigidbody2D::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
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
std::string ECS::Rigidbody2D::BindJit()
{
	std::string jit = R"(
ffi.cdef[[
	typedef enum {
        Static = 0,
		Kinematic = 1,
        Dynamic = 2,
    } MotionType;

	typedef struct 
	{ 
		MotionType type;
		bool active;
		bool kinematic;
		bool autoSleep;
		bool sensor;
		
		float gravity;
		float restitution;
		float friction;
		float linearDamping;

		Vector3 velocity;

		const bool lockPosition[3];
		const bool lockRotation[3];
		const uint32_t bodyID;
		const bool IsCreate;

	} Rigidbody2DData;
]]
    )";
	return jit;
}

void ECS::Rigidbody2D::SetPosition(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->SetPosition(JPH::BodyID(data->bodyID), x, y, z, true);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
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

void ECS::Rigidbody2D::AddImpulse(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->AddImpulse(JPH::BodyID(data->bodyID), x, y, z);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][AddImpulse] Not Find Component");
	}
}

void ECS::Rigidbody2D::AddForce(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->AddForce(JPH::BodyID(data->bodyID), x, y, z);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][AddForce] Not Find Component");
	}
}

void ECS::Rigidbody2D::LockPosition(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
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
	registry->AddComponent<Rigidbody2DData>(id);
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
