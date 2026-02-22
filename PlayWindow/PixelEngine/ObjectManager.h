#pragma once
#include "PixelObject.h"
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

	PPointer<GameObject> Get(std::string name);
	void Set(PPointer<GameObject> target);
	void ReloadLuaScript();
	PPointer<GameObject> Find(std::string name);

	void DeleteCheck();
private:
	std::vector<PPointer<GameObject>> Object_Run;
	std::queue<PPointer<GameObject>> Object_Idle;
	std::queue<PPointer<GameObject>> Object_Delete;
	size_t hashCode;
};

