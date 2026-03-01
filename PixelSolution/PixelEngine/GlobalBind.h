#pragma once
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>
#include "Type/GlobalEnum.h"
#include "Core/Module.h"
#include "Log.h"
#include "Memory.h"

class Transform;
class PVector3;
class GameObject;
class Camera;
class Renderer2D;
class LuaScript;
class BoxCollision2D;
class Collision2D;

#define BIND_TYPE_NAME(Type,Name) \
template <> struct TypeName<Type> { static std::string Get() { return Name; } }; 

template <typename T>
struct TypeName
{
	static std::string Get()
	{
		return typeid(T).name();
	}
};

BIND_TYPE_NAME(int, "number")
BIND_TYPE_NAME(double, "number")
BIND_TYPE_NAME(float, "number")
BIND_TYPE_NAME(std::string, "string")
BIND_TYPE_NAME(Transform, "Transform")
BIND_TYPE_NAME(PVector3, "Vector3")
BIND_TYPE_NAME(GameObject, "GameObject")
BIND_TYPE_NAME(Camera, "Camera")
BIND_TYPE_NAME(Renderer2D, "Renderer2D")
BIND_TYPE_NAME(LuaScript, "LuaScript")
BIND_TYPE_NAME(BoxCollision2D, "BoxCollision2D")
BIND_TYPE_NAME(Collision2D, "Collision2D")



struct ModuleTypeInfo 
{
	MODULE_TYPE type;
	std::string TypeName;
	std::string LuaTypeName;
	bool functionCheck[6] = { false, };
	std::function<Module*(void)> CreateModule;
};

class ModuleTypeManager
{
public:
	static ModuleTypeInfo GetTypeInfo(MODULE_TYPE type)
	{
		auto info = ModuleTypeManager::bindMap.find(type);
		if (info != ModuleTypeManager::bindMap.end())
		{
			return info->second;
		}
		else
		{
			Log::Error("Not Find ModuleType");
			return ModuleTypeInfo();
		}
	}
	static std::unordered_map<MODULE_TYPE, ModuleTypeInfo> bindMap;
};




template<std::derived_from<Module> T>
class ModuleTypeBind
{
public:
	ModuleTypeBind() = default;
	~ModuleTypeBind() = default;
	static void Bind(MODULE_TYPE type)
	{
		ModuleTypeInfo info;
		info.TypeName = ModuleTypeBind<T>::GetType();
		info.LuaTypeName = ModuleTypeBind<T>::GetLuaType();
		info.type = type;
		constexpr bool check[6] = 
		{
			!std::is_same_v<decltype(&T::Awake), decltype(&Module::Awake)>,
			!std::is_same_v<decltype(&T::Start), decltype(&Module::Start)>,
			!std::is_same_v<decltype(&T::Update), decltype(&Module::Update)>,
			!std::is_same_v<decltype(&T::MatrixUpdate), decltype(&Module::MatrixUpdate)>,
			!std::is_same_v<decltype(&T::PhysicsUpdate), decltype(&Module::PhysicsUpdate)>,
			!std::is_same_v<decltype(&T::LastUpdate), decltype(&Module::LastUpdate)>,
		};
		for (size_t i = 0; i < 6; i++) info.functionCheck[i] = check[i];
		info.CreateModule = []()
			{
				return PE_NEW T();
			};
		ModuleTypeManager::bindMap.insert({ type ,info });
	}
private:
	static std::string GetType()
	{
		std::string name = typeid(T).name();
		if (name.find("class ") == 0) name = name.substr(6);
		if (name.find("struct ") == 0) name = name.substr(7);
		return name;
	}
	static std::string GetLuaType()
	{
		std::string name = typeid(T).name();
		if (name == "int") return "number";
		if (name == "double") return "number";
		if (name == "float") return "number";
		if (name == "string") return "string";
		if (name.find("class ") == 0) name = name.substr(6);
		if (name.find("struct ") == 0) name = name.substr(7);
		return name;
	}
};




