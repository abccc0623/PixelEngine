#include "GameObject.h"
#include "Module.h"
#include "PixelEngine.h"
#include "BindManager.h"
#include <functional>
#include <iostream>
extern PixelEngine* Engine;
extern std::unordered_map<std::string, std::function<void(GameObject*)>> addModuleFunction;
GameObject::GameObject()
{
	ModuleMap = std::unordered_map<std::string, Module*>();
	hashCode = std::hash<GameObject*>{}(this);

	bindManager = Engine->GetFactory<BindManager>();
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

void GameObject::AddFunction(Module* target, int Type)
{
	Engine->RegisterFunction(this, target, Type);
	
}
