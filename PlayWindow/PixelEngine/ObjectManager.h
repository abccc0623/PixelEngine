#pragma once
#include "PixelObject.h"
#include "EngineManager.h"
#include <vector>
#include <queue>
class GameObject;
class ObjectManager : public EngineManager, PixelObject
{
public:
	ObjectManager();
	~ObjectManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	PPointer<GameObject> Get();
	void Set(PPointer<GameObject> target);
private:
	std::vector<PPointer<GameObject>> Object_Run;
	std::queue<PPointer<GameObject>> Object_Idle;
	size_t hashCode;
};

