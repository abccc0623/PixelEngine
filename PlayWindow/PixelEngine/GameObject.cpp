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
	ModuleMap = std::unordered_map<std::string, PPointer<Module>>();
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
	//해당 게임 오브젝트가 지워질때 내부의 모듈도 깨끗이 삭제
	functionManager->RemoveFunction(this);
	ModuleMap.clear();
}

size_t GameObject::GetHashCode()
{
	return hashCode;
}

std::vector<PPointer<Module>> GameObject::GetModules()
{
	std::vector<PPointer<Module>> list = std::vector<PPointer<Module>>();
	for (auto K : ModuleMap)
	{
		list.push_back(K.second);
	}
	return list;
}

void GameObject::Destroy()
{
	objectManager->Set(this);
}

void GameObject::ClearModules()
{
	ModuleMap.clear();
}

void GameObject::AddFunction(PPointer<Module> target, int Type)
{
	functionManager->AddFunction(this, target, Type);
}
