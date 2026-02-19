#pragma once
#include "EngineManager.h"
#include <vector>
#include <queue>
class GameObject;
class ObjectManager : public EngineManager
{
public:
	ObjectManager();
	~ObjectManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	GameObject* Get();
	void Set(GameObject* target);
private:
	std::vector<GameObject*> Object_Run;
	std::queue<GameObject*> Object_Idle;
	GameObject* CreateGameObject();
	size_t hashCode;
};

