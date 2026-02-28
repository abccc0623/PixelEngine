#pragma once
#include "Core/PixelObject.h"
#include "EngineManager.h"
#include <vector>
#include <queue>

template <typename T> class SPointer;
class FunctionManager;
class SceneManager;
class GameObject;

class ObjectManager : public EngineManager
{
public:
	ObjectManager();
	~ObjectManager();

	void Initialize() override;
	void Update() override;
	void ReleaseShared() override;

	SPointer<GameObject> Get(std::string name);
	void Set(SPointer<GameObject> target);
	void ReloadLuaScript();
	SPointer<GameObject> Find(std::string name);
	void DeleteCheck();


	SPointer<GameObject> Create(std::string name = "GameObject");
private:
	FunctionManager* functionManager = nullptr;
	SceneManager* sceneManager = nullptr;
};

