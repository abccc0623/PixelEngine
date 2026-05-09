#include "pch.h"
#include "GameObject.h"
#include "PixelEngine.h"
#include "FunctionManager.h"
#include "SPointer.h"
#include "GlobalBind.h"
#include "PixelMetaAPI.h"

#include "Core/Module.h"
#include "Module/Transform.h"
#include "GenerateLuaBind.h"

extern PixelEngine* Engine;
FunctionManager* GameObject::functionManager = nullptr;
std::unordered_map<std::string, std::function<sol::object(sol::this_state s, Module* target)>> AddModuleList;

GameObject::GameObject()
{
	ModuleMap = std::unordered_map<uint64_t, SPointer<Module>>();
	hashCode = std::hash<GameObject*>{}(this);
	if (functionManager == nullptr)
	{
		functionManager = Engine->GetFactory<FunctionManager>();
	}
}
GameObject::~GameObject()
{
	ModuleMap.clear();
}

bool GameObject::HasModule(PType* moduleClass)
{
	uint64_t hash = GetTypeHash(moduleClass);
	auto k = ModuleMap.find(hash);
	if ((k == ModuleMap.end()))
	{
		return false;
	}
	else
	{
		return true;
	}
}

Module* GameObject::GetModuleToEngine(PType* moduleClass)
{
	if (HasModule(moduleClass))
	{
		uint64_t hash = GetTypeHash(moduleClass);
		return ModuleMap[hash].GetPtr();
	}
	return nullptr;
}

Module* GameObject::AddModuleToEngine(const std::string& moduleName)
{
	auto targetType = GetType(moduleName);
	return AddModuleSetting(targetType);
}

Module* GameObject::AddModuleToEngine(PType* moduleClass)
{
	return AddModuleSetting(moduleClass);
}

Module* GameObject::GetModuleToEngine(const std::string& moduleName)
{
	auto targetType = GetType(moduleName);
	if (HasModule(targetType))
	{
		uint64_t hash = GetTypeHash(targetType);
		return ModuleMap[hash].GetPtr();
	}
	return nullptr;
}

bool GameObject::HasModuleToEngine(const std::string& moduleName)
{
	auto targetType = GetType(moduleName);
	return HasModule(targetType);
}

//Transform* GameObject::GetTransform()
//{
//	uint64_t hash = GetTypeHashByName("Transform");
//	return static_cast<Transform*>(ModuleMap[hash].GetPtr());
//}

unsigned long GameObject::GetHash()
{
	return hashCode;

}

Module* GameObject::AddModuleSetting(PType* moduleClass)
{
	if (HasModule(moduleClass) == true) return nullptr;
	if (GetTypeParentByHash(moduleClass) != GetTypeHashByName("Module")) return nullptr;

	//생성 함수 호출하여 모듈 생성
	std::vector<void*> property;
	PValue target = CallMethod(moduleClass, 0, moduleClass, property);
	Module* targetModule = reinterpret_cast<Module*>(target.AsPointer());
	auto module = SPointer<Module>(targetModule);

	//함수 등록
	int MethodCount = GetMethodCount(moduleClass);
	for (int i = 0; i < MethodCount; i++)
	{
		std::string name = GetMethodName(moduleClass, i);
		if (name == "Awake")
		{
			functionManager->AddOneTimeFunction(module, (int)MODULE_FUNC::AWAKE);
		}
		else if (name == "Start")
		{
			functionManager->AddOneTimeFunction(module, (int)MODULE_FUNC::START);
		}
		else if (name == "Update")
		{
			functionManager->AddTickFunction(module, (int)MODULE_FUNC::UPDATE);
		}
		else if (name == "MatrixUpdate")
		{
			functionManager->AddTickFunction(module, (int)MODULE_FUNC::MATRIX);
		}
		else if (name == "PhysicsUpdate")
		{
			functionManager->AddTickFunction(module, (int)MODULE_FUNC::PHYSICS);
		}
		else if (name == "LastUpdate")
		{
			functionManager->AddTickFunction(module, (int)MODULE_FUNC::LAST);
		}
	}
	ModuleMap.insert({ GetTypeHash(moduleClass) ,module });
	targetModule->targetObject = this;

	//기본 Transform 넣어준다
	auto hash = StringToByHash("Transform");
	auto find = ModuleMap.find(hash);
	if (find != ModuleMap.end())
	{
		//targetModule->transform = reinterpret_cast<Transform*>(ModuleMap[hash].GetPtr());
	}
	return targetModule;
}

sol::object GameObject::AddModule(sol::this_state s,std::string moduleName)
{
	PType* targetClass = GetType(moduleName);
	Module* targetModule = AddModuleToEngine(targetClass);
	
	auto find = AddModuleList.find(moduleName);
	if (find != AddModuleList.end())
	{
		return find->second(s, targetModule);
	}
	return sol::object();
}

sol::object GameObject::GetModule(sol::this_state s, std::string moduleName)
{
	PType* targetClass = GetType(moduleName);
	Module* targetModule = GetModuleToEngine(targetClass);

	auto find = AddModuleList.find(moduleName);
	if (find != AddModuleList.end())
	{
		return find->second(s, targetModule);
	}
	return sol::object();
}


