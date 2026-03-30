#pragma once
#include <unordered_map>
#include <string>
#include <sol/forward.hpp>
#include "Core/PixelObject.h"
#include "PixelEngineDLL.h"

template <typename T> class SPointer;
class Module;
class FunctionManager;
class PClass;
class Transform;
class GameObject :public PixelObject
{
public:
	GameObject();
	virtual ~GameObject();
public:
	 sol::object AddModule(sol::this_state s,std::string name);
	 sol::object GetModule(sol::this_state s, std::string name);

	 bool HasModule(PClass* moduleClass);
	 Module* AddModuleToEngine(PClass* moduleClass);
	 Module* GetModuleToEngine(PClass* moduleClass);
	 Transform* GetTransform();
	 unsigned long GetHash();

	//sol::object AddModuleToLua(std::string moduleName);
	std::string name;
private:
	unsigned long hashCode;
	std::unordered_map<uint64_t,SPointer<Module>> ModuleMap;
private:
	static FunctionManager* functionManager;
	friend FunctionManager;
};

