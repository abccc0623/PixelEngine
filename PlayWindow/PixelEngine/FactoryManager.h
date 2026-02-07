#pragma once
#include "EngineManager.h"
#include <vector>
#include <queue>
class GameObject;
class FactoryManager : public EngineManager
{
public:
	FactoryManager();
	~FactoryManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	GameObject* Get();
private:
	std::vector<GameObject*> Object_Run;
	std::queue<GameObject*> Object_Idle;
	GameObject* CreateGameObject();
	size_t hashCode;
};

