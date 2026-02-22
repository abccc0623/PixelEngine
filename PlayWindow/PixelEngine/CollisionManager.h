#pragma once
#include "EngineManager.h"
#include "PPointer.h"
#include "WPointer.h"
#include <vector>
class BoxCollision2D;
class CollisionManager : public EngineManager
{
public:
	CollisionManager();
	~CollisionManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	void CollisionUpdate();

	void Push(PPointer<BoxCollision2D> target);
private:
	std::vector<WPointer<BoxCollision2D>> CollisionList;

};

