#include "pch.h"
#include "ObjectManager.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Module/LuaScript.h"


#include "PixelEngine.h"
#include "FunctionManager.h"
#include "SceneManager.h"
#include "SPointer.h"


extern PixelEngine* Engine;
ObjectManager::ObjectManager()
{
	
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
	
}

void ObjectManager::ReleaseShared()
{

}

SPointer<GameObject> ObjectManager::Get(std::string name)
{
	return nullptr;
}

void ObjectManager::Set(SPointer<GameObject> target)
{

}

void ObjectManager::ReloadLuaScript()
{
	
}

SPointer<GameObject> ObjectManager::Find(std::string name)
{
	return nullptr;
}

void ObjectManager::DeleteCheck()
{
	
}

SPointer<GameObject> ObjectManager::Create(std::string name)
{
	//새로운 게임 오브젝트 생성
	SPointer<GameObject> p = SPointer<GameObject>::Make_SPointer();
	//기본 모듈인 Transform 추가
	p->AddModule(MODULE_TYPE::Transform);
	//씬 매니저에 등록
	sceneManager->RegisterGameObject(p);
	return p;
}

