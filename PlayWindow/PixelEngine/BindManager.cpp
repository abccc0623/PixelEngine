#include "BindManager.h"
#include <iostream>

#include "GameObject.h"
#include "Transform.h"
#include "LuaScript.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "DebugCamera.h"
#include "Camera.h"
#include "Module.h"
#include "BoxCollision2D.h"

template <typename T>
concept CanRegisterLua = requires {
	{ T::RegisterLua() } -> std::same_as<void>;
};

template <typename T>
void TryRegister(std::string name) {
	if constexpr (CanRegisterLua<T>) {
		T::RegisterLua();
	}
}

#define BIND_MODULE(Type) \
addModuleFunction[#Type] = [](GameObject* Obj) {Obj->AddModule<Type>(); }; \
getModuleFunction[#Type] = [](GameObject* Obj) { return static_cast<Type*>(Obj->GetModule<Type>());}; \
getLuaModuleFunction.insert({ #Type, [](sol::this_state s, GameObject& obj, std::string name) -> sol::object \
{ \
    auto targetModule = obj.GetModule(name); \
    if (targetModule != nullptr) \
    { \
        return sol::make_object(s,static_cast<Type*>(targetModule)); \
    } \
    return sol::make_object(s, sol::nil); \
} \
}); \
TryRegister<Type>(#Type)\



BindManager::BindManager()
{
}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
	BIND_MODULE(Transform);
	BIND_MODULE(LuaScript);
	BIND_MODULE(Renderer);
	BIND_MODULE(Renderer2D);
	BIND_MODULE(DebugCamera);
	BIND_MODULE(Camera);
	BIND_MODULE(BoxCollision2D);
}

void BindManager::Update()
{

}

void BindManager::Release()
{

}

void BindManager::AddModuleCall(std::string typeName, GameObject* obj)
{
	auto findTarget = addModuleFunction.find(typeName);
	if (findTarget != addModuleFunction.end())
	{
		findTarget->second(obj);
	}
	else 
	{
		std::cout << "[AddModuleCall]Not Find Module" + typeName << std::endl;
	}
}

Module* BindManager::GetModuleCall(std::string typeName, GameObject* obj)
{
	auto findTarget = getModuleFunction.find(typeName);
	if (findTarget != getModuleFunction.end())
	{
		return findTarget->second(obj);
	}
	else 
	{
		std::cout << "[GetModuleCall]Not Find Module" + typeName << std::endl;
	}
	return nullptr;
}

sol::object BindManager::GetLua(sol::this_state s,GameObject& obj, std::string name)
{
	auto target = getLuaModuleFunction.find(name);
	if (target == getLuaModuleFunction.end())
	{
		std::cout << "[GetModuleCall] Not Find Module" << std::endl;
	}
	auto fuc = target->second(s, obj, name);
	return fuc;
}


