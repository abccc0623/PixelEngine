#include "pch.h"
#include "Rigidbody2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void ECS::Rigidbody2D::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Rigidbody2DData>(id);
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
