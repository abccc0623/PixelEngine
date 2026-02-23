#include "SceneManager.h"
#include "Scene.h"
#include "GameObject.h"
#include <iostream>
SceneManager::SceneManager() :
	targetScene(nullptr),
	sceneMap(std::unordered_map<std::string, Scene*>())
{

}

SceneManager::~SceneManager()
{

}

void SceneManager::Initialize()
{
	//SPointer<Scene> target = SPointer(new Scene);

}

void SceneManager::Update()
{
	if (targetScene != nullptr)
	{
		targetScene->Update();
	}

}

void SceneManager::ReleaseShared()
{

}

void SceneManager::ChangeScene(std::string name)
{
	auto it = sceneMap.find(name);
	if (it != sceneMap.end())
	{
		targetScene = it->second;
		targetScene->Initialize();
		targetScene->Start();
	}
	else
	{
		std::cout << "Not Find Scene:" + name << std::endl;
	}
}

void SceneManager::CreateScene(const std::string& name)
{
	auto it = sceneMap.find(name);
	if (it != sceneMap.end())
	{
		std::cout << "A scene with the same name already exists." << std::endl;
	}
	else
	{
		Scene* newScene = new Scene();
		sceneMap.insert({ name, newScene });
	}
}

void SceneManager::Register(SPointer<GameObject> newObject)
{
	ObjectList.insert({newObject->GetHashCode(),newObject});
}
