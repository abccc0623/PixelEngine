#pragma once
#include "Collision2D.h"
#include "Rect.h"
class GameObject;
class CollisionManager;
class Transform;
struct RenderingData;
class BoxCollision2D : public Collision2D
{
public:
	BoxCollision2D();
	~BoxCollision2D();

	void Awake() override;
	void Start() override;
	void PhysicsUpdate() override;
	void LastUpdate() override;

	void OnCollisionEnter2D(WPointer<GameObject> target) override;
	void OnCollision2D(WPointer<GameObject> target) override;
	void OnCollisionExit2D(WPointer<GameObject> target) override;
	

	void SetRect(float offsetX, float offsetY, float sizeX, float sizeY);
	Rect GetRect();
	static std::string RegisterLua();
private:
	CollisionManager* collision;
	RenderingData* rendering;
	Transform* transform;
	
	Rect rect;
	Rect originalRect;
};

