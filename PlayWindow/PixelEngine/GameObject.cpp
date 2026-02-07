#include "GameObject.h"
#include "Module.h"
#include "FactoryManager.h"
#include "PixelEngine.h"
extern PixelEngine* Engine;
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

void GameObject::AddFunction(Module* target, int Type)
{
	Engine->RegisterFunction(this, target, Type);
}
