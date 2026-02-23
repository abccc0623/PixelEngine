#include "ObjectManager.h"
#include "GameObject.h"
#include "Transform.h"
#include <string>
#include <functional>
#include <algorithm>
#include "LuaScript.h"


#include "PixelEngine.h"
#include "FunctionManager.h"
#include "SceneManager.h"

extern PixelEngine* Engine;
ObjectManager::ObjectManager()
{
	Object_Run = std::vector<SPointer<GameObject>>();
	Object_Idle = std::queue<SPointer<GameObject>>();
}

ObjectManager::~ObjectManager()
{

}

void ObjectManager::Initialize()
{
	functionManager = Engine->GetFactory<FunctionManager>();
	sceneManager	= Engine->GetFactory<SceneManager>();
}

void ObjectManager::Update()
{
	functionManager->FunctionUpdate();
}

void ObjectManager::ReleaseShared()
{
	//while (Object_Delete.empty() == false) 
	//{
	//	Object_Delete.pop();
	//}
	//while (Object_Idle.empty() == false)
	//{
	//	Object_Idle.pop();
	//}
}

SPointer<GameObject> ObjectManager::Get(std::string name)
{
	if (Object_Idle.empty())
	{
		Object_Idle.push(SPointer<GameObject>::Make_SPointer());
	}
	SPointer<GameObject> PixelObject = Object_Idle.front();
	Object_Idle.pop();
	Object_Run.push_back(PixelObject);
	//PixelObject->AddModule<Transform>();
	PixelObject->name = name;
	return PixelObject;
}

void ObjectManager::Set(SPointer<GameObject> target)
{
	auto it = std::find(Object_Run.begin(), Object_Run.end(), target);
	if (it != Object_Run.end())
	{
		Object_Delete.push(target);
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

SPointer<GameObject> ObjectManager::Find(std::string name)
{
	auto it = std::find_if(Object_Run.begin(), Object_Run.end(), [&](const SPointer<GameObject>& target)
		{
			return target->name == name;
		});

	if (it != Object_Run.end())
	{
		return *it;
	}
	else 
	{
		std::cout << "Not Find GameObject : " + name << std::endl;
		return nullptr;
	}
}

void ObjectManager::DeleteCheck()
{
	while (Object_Delete.empty() == false)
	{
		Object_Delete.pop();
	}
}

SPointer<GameObject>& ObjectManager::Create(std::string name)
{
	//새로운 게임 오브젝트 생성
	SPointer<GameObject> p = SPointer<GameObject>::Make_SPointer();
	//기본 모듈인 Transform 추가
	functionManager->Register<Transform>(p.GetPtr());
	//씬 매니저에 등록
	sceneManager->Register(p);
	return p;
}

