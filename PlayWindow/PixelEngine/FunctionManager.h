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

	void RegisterFunction(GameObject* obj,Module* module,int type);
	
	template <std::derived_from<Module> T>
	void RemoveFunction(GameObject* obj)
	{



	}
private:
	std::queue<std::function<void(void)>> awakeFunction;
	std::queue<std::function<void(void)>> startFunction;
	std::unordered_map<std::string, std::function<void(void)>> updateFunction;
	std::unordered_map<std::string, std::function<void(void)>> matrixFunction;
	std::unordered_map<std::string, std::function<void(void)>> physicsFunction;
	std::unordered_map<std::string, std::function<void(void)>> lastFunction;
};

