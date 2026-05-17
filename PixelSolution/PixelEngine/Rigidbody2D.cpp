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

		bool lockPositionX;
		bool lockPositionY;
		bool lockPositionZ;
		bool lockRotationX;
		bool lockRotationY;
		bool lockRotationZ;

		bool IsChange;
		bool IsCreate;

		Vector3 velocity;
	    Vector3 pos;
		Vector3 rot;
		uint32_t bodyID;

	} Rigidbody2DData;
]]
    )";
	return jit;
}


/*
void ECS::Rigidbody2D::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
}

void ECS::Rigidbody2D::SetMotionType(unsigned int id, MotionType type)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->type = type;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetPosition(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		if (data->IsCreate == true)
		{
			auto phys = Engine->GetFactory<PhysManager>();
			phys->SetPosition(JPH::BodyID(data->bodyID), x, y, z, true);
		}
		else
		{
			data->position = { x,y,z };
		}
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		if (data->IsCreate == true)
		{
			auto phys = Engine->GetFactory<PhysManager>();
			phys->SetRotation(JPH::BodyID(data->bodyID), x, y, z, true);
		}
		else
		{
			data->rotation = { x,y,z };
		}
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetRotation] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetKinematic(unsigned int id, bool Kinematic)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->Kinematic = Kinematic;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetKinematic] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetAutoSleep(unsigned int id, bool AutoSleep)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->AutoSleep = AutoSleep;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetAutoSleep] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetSensor(unsigned int id, bool Sensor)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->Sensor = Sensor;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetSensor] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetGravity(unsigned int id, float Gravity)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->Gravity = Gravity;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetGravity] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetRestitution(unsigned int id, float Restitution)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->Restitution = Restitution;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetRestitution] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetFriction(unsigned int id, float Friction)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->Friction = Friction;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetFriction] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetLinearDamping(unsigned int id, float LinearDamping)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->LinearDamping = LinearDamping;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetLinearDamping] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetPositionLock(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->LockPositionX = x;
		data->LockPositionY = y;
		data->LockPositionZ = z;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetPositionLock] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetRotationLock(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		data->LockRotationX = x;
		data->LockRotationY = y;
		data->LockRotationZ = z;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetRotationLock] Not Find Component");
	}
}
*/
