#pragma once
#include "EngineManager.h"

class PhysicsFunction : public EngineManager
{
public:
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void MoveKinematic(JPH::BodyID id, float x, float y, float z);
	void SetPosition(JPH::BodyID id, float x, float y, float z);
	void SetRotation(JPH::BodyID id, float x, float y, float z);
	void SetActive(JPH::BodyID id, bool active);
	void SetVelocity(JPH::BodyID id, float x, float y, float z);
	void SetMotionType(JPH::BodyID id, JPH::EMotionType type);
	void SetGravity(JPH::BodyID id, float Gravity);
	void SetRestitution(JPH::BodyID id, float restitution);
	void SetFriction(JPH::BodyID id, float friction);
	void SetLinearDamping(JPH::BodyID id, float linearDamping);
	void SetSensor(JPH::BodyID id, bool sensor);
	void SetLayer(JPH::BodyID id, const char* layerName);
private:

};

