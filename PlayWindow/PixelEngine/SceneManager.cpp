#include "SceneManager.h"
#include "Scene.h"
SceneManager::SceneManager() :
	targetScene(nullptr)
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

void SceneManager::ChangeScene(Scene* scene)
{
	if (targetScene != nullptr) 
	{
		targetScene->Release();
	}
	targetScene = scene;
	targetScene->Initialize();
	targetScene->Start();
}
