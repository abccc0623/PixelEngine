#include "GameObject.h"
#include "Module.h"
#include "PixelEngine.h"
#include "BindManager.h"
#include "FunctionManager.h"
#include "ObjectManager.h"
#include <functional>
#include <iostream>
extern PixelEngine* Engine;
BindManager* GameObject::bindManager = nullptr;
FunctionManager* GameObject::functionManager = nullptr;
ObjectManager* GameObject::objectManager = nullptr;

GameObject::GameObject()
{
	ModuleMap = std::unordered_map<std::string, Module*>();
	hashCode = std::hash<GameObject*>{}(this);

	if (bindManager == nullptr)
	{
		bindManager = Engine->GetFactory<BindManager>();
	}
	if (functionManager == nullptr)
	{
		functionManager = Engine->GetFactory<FunctionManager>();
	}
	if (objectManager == nullptr)
	{
		objectManager = Engine->GetFactory<ObjectManager>();
	}
}
GameObject::~GameObject()
{
	for (const auto& pair : ModuleMap)
	{
		delete pair.second;
	}
	ModuleMap.clear();
}

size_t GameObject::GetHashCode()
{
	return hashCode;
}

Module* GameObject::AddModule(std::string name)
{
	bindManager->AddModuleCall(name,this);
	return GetModule(name);
}

Module* GameObject::GetModule(std::string name)
{
	return bindManager->GetModuleCall(name, this);
}

std::vector<Module*> GameObject::GetModules()
{
	std::vector<Module*> list = std::vector<Module*>();
	for (auto K : ModuleMap)
	{
		list.push_back(K.second);
	}
	return list;
}

void GameObject::Destroy()
{
	functionManager->RemoveFunction(this);
}

void GameObject::ClearModules()
{
	ModuleMap.clear();
}

void GameObject::AddFunction(Module* target, int Type)
{
	functionManager->RegisterFunction(this, target, Type);
}
