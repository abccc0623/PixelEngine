#pragma once
#include "PixelObject.h"
#include "Module.h"
#include "FunctionManager.h"
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <concepts>
#include <vector>
#include <string>
#include "sol.hpp"

class BindManager;
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
	SPointer<T> AddModule()
	{
		functionManager->template Register<T>(this);
		return GetModule<T>();
	}

	template <std::derived_from<Module> T>
	SPointer<T> GetModule()
	{
		std::string nameKey = typeid(T).name();
		if (nameKey.find("class ") == 0) nameKey = nameKey.substr(6);
		auto findTarget = ModuleMap.find(nameKey);
		
		if (findTarget == ModuleMap.end()) return nullptr;
		return SPointer<T>(findTarget->second);
	}

	template <std::derived_from<Module> T>
	bool HasModule()
	{
		std::string nameKey = typeid(T).name();
		if (nameKey.find("class ") == 0) nameKey = nameKey.substr(6);
		auto findTarget = ModuleMap.find(nameKey);
		return (findTarget == ModuleMap.end()) ? false : true;
	}

	std::vector<SPointer<Module>> GetModules();
	void Destroy();
	void OnCollision2D(WPointer<GameObject> target);

	std::string Save(int tab);
private:
	size_t hashCode;
	std::unordered_map<std::string,SPointer<Module>> ModuleMap;
private:
	static BindManager* bindManager;
	static FunctionManager* functionManager;
	static ObjectManager* objectManager;
	friend FunctionManager;
};



/// 해당 함수는 Function 매니저 쪽에 추가
template <std::derived_from<Module> T>
inline void FunctionManager::Register(GameObject* target)
{
	SPointer<T> m = SPointer<T>::Make_SPointer();

	// 오버라이딩 체크 로직
	constexpr bool check[6] = {
		!std::is_same_v<decltype(&T::Awake), decltype(&Module::Awake)>,
		!std::is_same_v<decltype(&T::Start), decltype(&Module::Start)>,
		!std::is_same_v<decltype(&T::Update), decltype(&Module::Update)>,
		!std::is_same_v<decltype(&T::MatrixUpdate), decltype(&Module::MatrixUpdate)>,
		!std::is_same_v<decltype(&T::PhysicsUpdate), decltype(&Module::PhysicsUpdate)>,
		!std::is_same_v<decltype(&T::LastUpdate), decltype(&Module::LastUpdate)>,
	};

	m->targetObject = target;
	target->ModuleMap.insert({ m->GetClassNameString(), m });
	if constexpr (check[AWAKE_FUNCTION]) { AddOneTimeFunction(m, AWAKE_FUNCTION); }
	if constexpr (check[START_FUNCTION]) { AddOneTimeFunction(m, START_FUNCTION); }
	if constexpr (check[UPDATE_FUNCTION]) { AddTickFunction(m, UPDATE_FUNCTION); }
	if constexpr (check[MATRIX_UPDATE_FUNCTION]) { AddTickFunction(m, MATRIX_UPDATE_FUNCTION); }
	if constexpr (check[PHYSICS_UPDATE_FUNCTION]) { AddTickFunction(m, PHYSICS_UPDATE_FUNCTION); }
	if constexpr (check[LAST_UPDATE_FUNCTION]) { AddTickFunction(m, LAST_UPDATE_FUNCTION); }
}

