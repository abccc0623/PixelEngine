#pragma once
#include "EngineManager.h"
#include"GameObject.h"
#include <unordered_set>
#include <string>
#include <functional>
#include <queue>

class Action;
class GameObject;
class Module;

class FunctionManager : public EngineManager
{
public:
	FunctionManager();
	~FunctionManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	void AddFunction(GameObject* obj, PPointer<Module> module,int type);
	void RemoveFunction(GameObject* obj);
private:
	std::queue<Action*> awakeFunction;
	std::queue<Action*> startFunction;
	std::unordered_map<std::string, Action*> updateFunction;
	std::unordered_map<std::string, Action*> matrixFunction;
	std::unordered_map<std::string, Action*> physicsFunction;
	std::unordered_map<std::string, Action*> lastFunction;
	bool isAwakeCall = false;
	void RemoveFunction(std::unordered_map<std::string, Action*>& remove, std::string key);
	std::queue<GameObject*> RemoveList;

	std::queue<Action*> nextStartFunction;
};

