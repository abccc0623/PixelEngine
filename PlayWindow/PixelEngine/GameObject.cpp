#include "GameObject.h"
#include "Module.h"
#include "FactoryManager.h"
#include "PixelEngine.h"
#include <functional>
#include <iostream>
extern PixelEngine* Engine;
extern std::unordered_map<std::string, std::function<void(GameObject*)>> moduleFactories;
GameObject::GameObject()
{
	ModuleMap = std::unordered_map<std::string, Module*>();
	hashCode = std::hash<GameObject*>{}(this);
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
	auto findTarget = moduleFactories.find(name);
	if (findTarget != moduleFactories.end())
	{
		findTarget->second(this);
	}
	return GetModule(name);
}

Module* GameObject::GetModule(std::string name)
{
	auto findTarget = ModuleMap.find(name);
	if (findTarget != ModuleMap.end())
	{
		return findTarget->second;
	}
	else 
	{
		std::cout << "Not Find Module : "+ name << std::endl;
	}
	return nullptr;
}

void GameObject::AddFunction(Module* target, int Type)
{
	Engine->RegisterFunction(this, target, Type);
}
