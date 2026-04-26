#pragma once
#include "Collision2D.h"
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyID.h>  
#include "Rect.h"
class GameObject;
class PhysManager;
class Transform;
struct RenderingData;

namespace JPH 
{
	class Shape; 
}
class BoxCollider2D : public Module
{
public:
	BoxCollider2D();
	~BoxCollider2D();

	void Awake() override;
	void Start() override;
	void PhysicsUpdate() override;

	void SetCenter(float x, float y);
	void SetOffset(float x, float y);
	void SetGravity(float gravity);
	void SetCreateActive(bool active);
	void SetPhysType(int type);
	void CreatePhys();
private:
	void CreateCollider();
	void CreateBody();

	PhysManager* collision;
	RenderingData* rendering;
	Transform* transform;

	JPH::ShapeRefC targetRef;
	JPH::BodyID targetBodyID;
	float targetGravity;
	float offsetX = 0.5f;
	float offsetY = 0.5f;
	float centerX = 0.0f;
	float centerY = 0.0f;
	int motionType;
	bool active = true;
};

