#pragma once
#include "Core/Module.h"
#include "Physics2D.h"
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyID.h>  
#include <sol/forward.hpp>
#include <string>
#include "PhysStruct.h"
class GameObject;
class PhysManager;
struct RenderingData;

namespace JPH
{
	class Shape;
}
class Physics2D : public Module
{
public:
	Physics2D();
	~Physics2D();

	void Awake() override;
	void Start() override;
	void PhysicsUpdate() override;

	void SetActive(bool active);
	bool GetActive();

	void SetVelocity(float x, float y);
	void SetVelocityX(float x);
	void SetVelocityY(float y);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void AddForce(float x, float y, float z);
	void AddImpulse(float x, float y, float z);

	void SetCollider(sol::table table);
	void SetRigidbody(sol::table table);
private:
	PhysManager* collision;

	JPH::ShapeRefC targetRef;
	JPH::BodyID targetBodyID;

	PhysCollider colliderSetting;
	PhysRigidbody rigidbodySetting;
};

