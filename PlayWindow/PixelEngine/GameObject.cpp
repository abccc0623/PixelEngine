
#include "GameObject.h"
#include "Module.h"
#include "PixelEngine.h"
#include "BindManager.h"
#include "FunctionManager.h"
#include "ObjectManager.h"
#include <functional>
#include <iostream>
#include "Transform.h"
extern PixelEngine* Engine;
BindManager* GameObject::bindManager = nullptr;
FunctionManager* GameObject::functionManager = nullptr;
ObjectManager* GameObject::objectManager = nullptr;

GameObject::GameObject()
{
	ModuleMap = std::unordered_map<std::string, SPointer<Module>>();
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
	ModuleMap.clear();
}

size_t GameObject::GetHashCode()
{
	return hashCode;
}

std::vector<SPointer<Module>> GameObject::GetModules()
{
	std::vector<SPointer<Module>> list = std::vector<SPointer<Module>>();
	for (auto K : ModuleMap)
	{
		list.push_back(K.second);
	}
	return list;
}

void GameObject::Destroy()
{
	
}

void GameObject::ClearModules()
{
	ModuleMap.clear();
}

void GameObject::OnCollision2D(WPointer<GameObject> target)
{
	for (auto& K : ModuleMap)
	{
		if (K.second->isCollisionModule == true)continue;
		K.second->OnCollision2D(target);
	}
}

