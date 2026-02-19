#include "ObjectManager.h"
#include "GameObject.h"
#include "Transform.h"
#include <string>
#include <functional>
#include <algorithm>

ObjectManager::ObjectManager()
{
	Object_Run = std::vector<GameObject*>();
	Object_Idle = std::queue<GameObject*>();
}

ObjectManager::~ObjectManager()
{

}

void ObjectManager::Initialize()
{
	for(int  i = 0; i < 20; i++)
	{
		GameObject* target = CreateGameObject();
		Object_Idle.push(target);
	}
}

void ObjectManager::Update()
{
	
}

void ObjectManager::Release()
{
	while (Object_Idle.empty() == false)
	{
		GameObject* Obj = Object_Idle.front();
		Object_Idle.pop();
		delete Obj;
	}
}

GameObject* ObjectManager::Get()
{
	if (Object_Idle.empty())
	{
		for (int i = 0; i < 10; i++)
		{
			GameObject* Object = CreateGameObject();
			Object_Idle.push(Object);
		}
	}
	GameObject* Object = Object_Idle.front();
	Object_Idle.pop();
	Object_Run.push_back(Object);
	Object->AddModule<Transform>();
	return Object;
}

void ObjectManager::Set(GameObject* target)
{
	auto it = std::find(Object_Run.begin(), Object_Run.end(), target);
	if (it != Object_Run.end())
	{
		Object_Run.erase(it);
	}
}

GameObject* ObjectManager::CreateGameObject()
{
	GameObject* target = new GameObject();
	return target;
}
