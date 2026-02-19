#pragma once
#include "EngineManager.h"
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <string>
#include "sol.hpp"

class GameObject;
class Module;
class BindManager : public EngineManager 
{
public:
	BindManager();
	~BindManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	void AddModuleCall(std::string typeName, GameObject* obj);
	Module* GetModuleCall(std::string typeName, GameObject* obj);
	sol::object GetLua(sol::this_state s, GameObject& obj, std::string name);
private:
	std::unordered_map<std::string, std::function<void(GameObject*)>>		addModuleFunction;
	std::unordered_map<std::string, std::function<Module*(GameObject*)>>	getModuleFunction;
	std::unordered_map<std::string, std::function<sol::object(sol::this_state, GameObject&, std::string)>>	getLuaModuleFunction;
};