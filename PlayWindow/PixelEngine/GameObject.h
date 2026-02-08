#pragma once
#include "Object.h"
#include "Module.h"
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <concepts>
#include <vector>
#include <string>
#include "sol.hpp"
#define AWAKE_FUNCTION 0
#define START_FUNCTION 1
#define UPDATE_FUNCTION 2
#define MATRIX_UPDATE_FUNCTION 3
#define PHYSICS_UPDATE_FUNCTION 4
#define LAST_UPDATE_FUNCTION 5
class Module;
class GameObject :public Object
{
public:
	GameObject();
	~GameObject();
public:
	size_t GetHashCode();

	template <std::derived_from<Module> T>
	T* AddModule() 
	{
		T* target = new T();
		constexpr bool check[6] =
		{
			!std::is_same_v<decltype(&T::Awake), decltype(&Module::Awake)>,
			!std::is_same_v<decltype(&T::Start), decltype(&Module::Start)>,
			!std::is_same_v<decltype(&T::Update), decltype(&Module::Update)>,
			!std::is_same_v<decltype(&T::MatrixUpdate), decltype(&Module::MatrixUpdate)>,
			!std::is_same_v<decltype(&T::PhysicsUpdate), decltype(&Module::PhysicsUpdate)>,
			!std::is_same_v<decltype(&T::LastUpdate), decltype(&Module::LastUpdate)>,
		};
		if constexpr (check[AWAKE_FUNCTION]) { AddFunction(target, AWAKE_FUNCTION); }
		if constexpr (check[START_FUNCTION]) { AddFunction(target, START_FUNCTION); }
		if constexpr (check[UPDATE_FUNCTION]) {AddFunction(target, UPDATE_FUNCTION);}
		if constexpr (check[MATRIX_UPDATE_FUNCTION]) { AddFunction(target, MATRIX_UPDATE_FUNCTION); }
		if constexpr (check[PHYSICS_UPDATE_FUNCTION]) { AddFunction(target, PHYSICS_UPDATE_FUNCTION); }
		if constexpr (check[LAST_UPDATE_FUNCTION]) {AddFunction(target, LAST_UPDATE_FUNCTION);}
		auto name = target->GetClassNameString();
		target->targetObject = this;
		ModuleMap.insert({ target->GetClassNameString(),target });
		return target;
	}

	template <std::derived_from<Module> T>
	T* GetModule()
	{
		std::string nameKey = typeid(T).name();
		if (nameKey.find("class ") == 0) nameKey = nameKey.substr(6);
		auto findTarget = ModuleMap.find(nameKey);

		if (findTarget == ModuleMap.end()) return nullptr;
		
		Module* basePtr = findTarget->second;
		return static_cast<T*>(basePtr);
	}
	Module* AddModule(std::string name);
	Module* GetModule(std::string name);
private:
	size_t hashCode;
	void AddFunction(Module* target, int Type);
	std::unordered_map<std::string,Module*> ModuleMap;
};

