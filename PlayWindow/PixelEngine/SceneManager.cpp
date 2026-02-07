#include "SceneManager.h"
#include "Scene.h"
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
	
}

void SceneManager::Update()
{
	if (targetScene != nullptr)
	{
		targetScene->Update();
	}

}

void SceneManager::Release()
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

void SceneManager::CreateScene(std::string name)
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
