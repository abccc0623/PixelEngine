#include "ObjectManager.h"
#include "GameObject.h"
#include "Transform.h"
#include <string>
#include <functional>
#include <algorithm>
#include "LuaScript.h"

ObjectManager::ObjectManager()
{
	Object_Run = std::vector<PPointer<GameObject>>();
	Object_Idle = std::queue<PPointer<GameObject>>();
}

ObjectManager::~ObjectManager()
{

}

void ObjectManager::Initialize()
{
	
}

void ObjectManager::Update()
{
	
}

void ObjectManager::Release()
{

}

PPointer<GameObject> ObjectManager::Get(std::string name)
{
	if (Object_Idle.empty())
	{
		Object_Idle.push(MakePixel<GameObject>());
	}
	PPointer<GameObject> PixelObject = Object_Idle.front();
	Object_Idle.pop();
	Object_Run.push_back(PixelObject);
	PixelObject->AddModule<Transform>();
	PixelObject->name = name;
	return PixelObject;
}

void ObjectManager::Set(PPointer<GameObject> target)
{
	auto it = std::find(Object_Run.begin(), Object_Run.end(), target);
	if (it != Object_Run.end())
	{
		Object_Run.erase(it);
	}
}

void ObjectManager::ReloadLuaScript()
{
	for (auto K : Object_Run)
	{
		if (K->HasModule<LuaScript>())
		{
			auto lua = K->GetModule<LuaScript>();
			lua->Reload();
		}
	}
}

