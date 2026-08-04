#include "pch.h"
#include "PhysicsFunction.h"
#include "PhysManager.h"
#include "PixelEngineAPI.h"
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include "PixelObjectLayerPairFilter.h"

void PhysicsFunction::Initialize()
{

}

void PhysicsFunction::Update()
{

}

void PhysicsFunction::Release()
{

}

void PhysicsFunction::Clear()
{

}

void PhysicsFunction::MoveKinematic(JPH::BodyID id, float x, float y, float z)
{
	JPH::Vec3 targetPosition(x, y, 0);
	PhysManager::mBodyInterface->MoveKinematic(id, targetPosition, JPH::Quat::sIdentity(), GetDeltaTime());
}

void PhysicsFunction::SetPosition(JPH::BodyID id, float x, float y, float z)
{
	JPH::Vec3 targetPosition(x, y, 0);
	PhysManager::mBodyInterface->SetPosition(id, targetPosition, JPH::EActivation::Activate);
}

void PhysicsFunction::SetRotation(JPH::BodyID id, float x, float y, float z)
{
	constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
	JPH::Vec3 eulerRadians(x * DEG_TO_RAD, y * DEG_TO_RAD, z * DEG_TO_RAD);
	JPH::Quat targetRotation = JPH::Quat::sEulerAngles(eulerRadians);
	PhysManager::mBodyInterface->SetRotation(
		id,
		targetRotation,
		JPH::EActivation::Activate
	);
}

void PhysicsFunction::SetActive(JPH::BodyID id, bool active)
{
	if (id.IsInvalid()) return;
	if (active)
	{
		if (PhysManager::mBodyInterface->IsAdded(id) == false)
		{
			PhysManager::mBodyInterface->AddBody(id, JPH::EActivation::Activate);
		}
	}
	else
	{
		if (PhysManager::mBodyInterface->IsAdded(id) == true)
		{
			SetVelocity(id, 0, 0, 0);
			PhysManager::mBodyInterface->RemoveBody(id);
		}
	}
}

void PhysicsFunction::SetVelocity(JPH::BodyID id, float x, float y, float z)
{
	JPH::Vec3 targetVelocity(x, y, 0.0f);
	PhysManager::mBodyInterface->SetLinearVelocity(id, targetVelocity);
}

void PhysicsFunction::SetMotionType(JPH::BodyID id, JPH::EMotionType type)
{
	PhysManager::mBodyInterface->SetMotionType(id, type, JPH::EActivation::Activate);
}

void PhysicsFunction::SetGravity(JPH::BodyID id, float Gravity)
{
	PhysManager::mBodyInterface->SetGravityFactor(id, Gravity);
}

void PhysicsFunction::SetRestitution(JPH::BodyID id, float restitution)
{
	PhysManager::mBodyInterface->SetRestitution(id, restitution);
}

void PhysicsFunction::SetFriction(JPH::BodyID id, float friction)
{
	PhysManager::mBodyInterface->SetFriction(id, friction);
}

void PhysicsFunction::SetLinearDamping(JPH::BodyID id, float linearDamping)
{
	if (id.IsInvalid() || PhysManager::mBodyLockInterface == nullptr)
	{
		return;
	}

	JPH::BodyLockWrite lock(
		*PhysManager::mBodyLockInterface,
		id);
	if (!lock.Succeeded())
	{
		return;
	}

	JPH::MotionProperties* motionProperties =
		lock.GetBody().GetMotionProperties();
	if (motionProperties == nullptr)
	{
		return;
	}

	motionProperties->SetLinearDamping(
		linearDamping < 0.0f ? 0.0f : linearDamping);
}

void PhysicsFunction::SetSensor(JPH::BodyID id, bool sensor)
{
	PhysManager::mBodyInterface->SetIsSensor(id, sensor);
}

void PhysicsFunction::SetLayer(JPH::BodyID id, const char* layerName)
{
	std::string layerNameSTR(layerName);
	int size = Layers::layerNames.size();
	for (int i = 0; i < size; i++)
	{
		if (Layers::layerNames[i] == layerNameSTR)
		{
			PhysManager::mBodyInterface->SetObjectLayer(id, JPH::ObjectLayer(i));
			break;
		}
	}
}

