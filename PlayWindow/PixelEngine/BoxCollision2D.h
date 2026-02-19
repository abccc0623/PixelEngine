#pragma once
#include "Module.h"
class CollisionManager;
class BoxCollision2D : public Module
{
public:
	BoxCollision2D();
	~BoxCollision2D();

	void Awake() override;
	void Start() override;
	void PhysicsUpdate() override;
	static void RegisterLua();
private:
	CollisionManager* collision;

};

