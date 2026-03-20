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

bool GameObject::HasModule(PClass* moduleClass)
{
	uint64_t hash = GetClassHash(moduleClass);
	auto k = ModuleMap.find(hash);
	return (k == ModuleMap.end()) ? false : true;
}

Module* GameObject::GetModuleToEngine(PClass* moduleClass)
{
	if (HasModule(moduleClass))
	{
		uint64_t hash = GetClassHash(moduleClass);
		return ModuleMap[hash].GetPtr();
	}
	return nullptr;
}

unsigned long GameObject::GetHash()
{
	return hashCode;

}

sol::object GameObject::AddModule(sol::this_state s,std::string moduleName)
{
	PClass* targetClass = GetClass(moduleName);
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
	PClass* targetClass = GetClass(moduleName);
	Module* targetModule = GetModuleToEngine(targetClass);

	auto find = AddModuleList.find(moduleName);
	if (find != AddModuleList.end())
	{
		return find->second(s, targetModule);
	}
	return sol::object();
}

Module* GameObject::AddModuleToEngine(PClass* moduleClass)
{
	if (HasModule(moduleClass) == true) return nullptr;
	if (GetClassParentHash(moduleClass) != GetClassHashByString("Module")) return nullptr;
	
	//생성 함수 호출하여 모듈 생성
	std::vector<void*> property;
	PValue target = CallClassMethod(moduleClass, 0, moduleClass, property);
	Module* targetModule = reinterpret_cast<Module*>(target.AsPointer());
	auto module =  SPointer<Module>(targetModule);

	//함수 등록
	int MethodCount = GetClassMethodCount(moduleClass);
	for (int i = 0; i < MethodCount; i++)
	{
		std::string name = GetClassMethodName(moduleClass, i);
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
	ModuleMap.insert({ GetClassHash(moduleClass) ,module});
	targetModule->targetObject = this;
	
	//기본 Transform 넣어준다
	auto hash = StringToByHash("Transform");
	auto find =  ModuleMap.find(hash);
	if (find != ModuleMap.end())
	{
		targetModule->transform = reinterpret_cast<Transform*>(ModuleMap[hash].GetPtr());
	}
	return targetModule;
}
