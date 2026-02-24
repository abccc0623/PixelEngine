#include "SceneManager.h"
#include "Scene.h"
#include "PixelObject.h"
#include "GameObject.h"
#include <iostream>
#include <filesystem>
SceneManager::SceneManager() 
{
	nowScene = nullptr;
	SceneMap = std::unordered_map<std::string, SPointer<Scene>>();
}

SceneManager::~SceneManager()
{

}

void SceneManager::Initialize()
{

}

void SceneManager::Update()
{
	if (nowScene.IsValid())
	{
		SPointer<Scene> p = nowScene.Lock();
		p->Update();
	}
}

void SceneManager::ReleaseShared()
{

}

void SceneManager::ChangeScene(std::string name)
{
	auto it = SceneMap.find(name);
	if (it != SceneMap.end())
	{
		//이전 씬이 있다면 정리
		if (nowScene != nullptr)
		{
			if (nowScene.IsValid()) 
			{
				SPointer<Scene> p = nowScene.Lock();
				p->Release();
			}
		}
		//씬 변경
		nowScene = SceneMap[name];
		
		//현재 씬 초기화
		if (nowScene.IsValid())
		{
			if (nowScene.IsValid())
			{
				SPointer<Scene> p = nowScene.Lock();
				p->Start();
			}
		}
	}
	else
	{
		std::cout << "Not Find Scene :"+ name << std::endl;
	}
}

void SceneManager::CreateScene(const std::string& luaPath)
{
	std::filesystem::path p(luaPath);

	std::string directory = p.parent_path().string(); 
	std::string fileName = p.filename().string();    
	std::string stem = p.stem().string();            
	std::string extension = p.extension().string();  

	auto find = SceneMap.find(stem);
	if (find == SceneMap.end())
	{
		auto s = SPointer<Scene>::Make_SPointer();
		s->Initialize(luaPath, stem);
		SceneMap.insert({ stem,s });
	}
	else 
	{
		std::cout << "This scene name is already in use :" + stem << std::endl;
	}
}

void SceneManager::Register(SPointer<GameObject>& newObject)
{
	if (nowScene.IsValid())
	{
		auto block = nowScene.Lock();
		block->CreateGameObject(newObject);
	}
}
