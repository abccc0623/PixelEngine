#include "ObjectManager.h"
#include "GameObject.h"
#include "Transform.h"
#include <string>
#include <functional>
#include <algorithm>

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
	for(int  i = 0; i < 20; i++)
	{
		PPointer<GameObject> target = MakePixel<GameObject>();
		Object_Idle.push(target);
	}
}

void ObjectManager::Update()
{
	
}

void ObjectManager::Release()
{

}

PPointer<GameObject> ObjectManager::Get()
{
	if (Object_Idle.empty())
	{
		for (int i = 0; i < 10; i++)
		{
			Object_Idle.push(MakePixel<GameObject>());
		}
	}
	PPointer<GameObject> PixelObject = Object_Idle.front();
	Object_Idle.pop();
	Object_Run.push_back(PixelObject);
	PixelObject->AddModule<Transform>();
	return PixelObject;
}

void ObjectManager::Set(PPointer<GameObject> target)
{
	auto it = std::find(Object_Run.begin(), Object_Run.end(), target);
	if (it != Object_Run.end())
	{
		Object_Run.erase(it);
		Object_Idle.push(target);
	}
}

