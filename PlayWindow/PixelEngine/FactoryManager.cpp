#include "FactoryManager.h"
#include "GameObject.h"
#include "Transform.h"
#include <string>
#include <functional>

FactoryManager::FactoryManager()
{
	Object_Run = std::vector<GameObject*>();
	Object_Idle = std::queue<GameObject*>();
}

FactoryManager::~FactoryManager()
{

}

void FactoryManager::Initialize()
{
	for(int  i = 0; i < 20; i++)
	{
		GameObject* target = CreateGameObject();
		Object_Idle.push(target);
	}
}

void FactoryManager::Update()
{
	
}

void FactoryManager::Release()
{
	while (Object_Idle.empty() == false)
	{
		GameObject* Obj = Object_Idle.front();
		Object_Idle.pop();
		delete Obj;
	}
}

GameObject* FactoryManager::Get()
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

GameObject* FactoryManager::CreateGameObject()
{
	GameObject* target = new GameObject();
	return target;
}
