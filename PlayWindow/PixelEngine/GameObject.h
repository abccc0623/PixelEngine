#pragma once
#include "PixelObject.h"
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

class BindManager;
class FunctionManager;
class ObjectManager;
class Module;
class GameObject :public PixelObject
{
public:
	GameObject();
	virtual ~GameObject();
public:
	size_t GetHashCode();
	std::string name;

	template <std::derived_from<Module> T>
	PPointer<T> AddModule()
	{
		//이미 있다면 
		if (HasModule<T>()){return GetModule<T>();}

		PPointer<T> target = MakePixel<T>();
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
	PPointer<T> GetModule()
	{
		std::string nameKey = typeid(T).name();
		if (nameKey.find("class ") == 0) nameKey = nameKey.substr(6);
		auto findTarget = ModuleMap.find(nameKey);
		
		if (findTarget == ModuleMap.end()) return nullptr;
		return PPointer<T>(findTarget->second);
	}

	template <std::derived_from<Module> T>
	bool HasModule()
	{
		std::string nameKey = typeid(T).name();
		if (nameKey.find("class ") == 0) nameKey = nameKey.substr(6);
		auto findTarget = ModuleMap.find(nameKey);
		return (findTarget == ModuleMap.end()) ? false : true;
	}

	std::vector<PPointer<Module>> GetModules();
	void Destroy();
	void ClearModules();
	void OnCollision2D(WPointer<GameObject> target);
private:
	size_t hashCode;
	void AddFunction(PPointer<Module> target, int Type);
	std::unordered_map<std::string,PPointer<Module>> ModuleMap;
private:
	static BindManager* bindManager;
	static FunctionManager* functionManager;
	static ObjectManager* objectManager;
};

