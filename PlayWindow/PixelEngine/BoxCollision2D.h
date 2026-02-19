#pragma once
#include "Module.h"
class CollisionManager;
class Transform;
struct RenderingData;
class BoxCollision2D : public Module
{
public:
	BoxCollision2D();
	~BoxCollision2D();

	void Awake() override;
	void Start() override;
	void PhysicsUpdate() override;
	void LastUpdate() override;

	void SetRect(float offsetX, float offsetY, float sizeX, float sizeY);
	static void RegisterLua();
private:
	CollisionManager* collision;
	RenderingData* rendering;
	Transform* transform;
};

