#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Log.h"
#include "Core/PixelObject.h"
#include "Core/GameObject.h"
#include "JsonManager.h"
#include "PixelEngine.h"

extern PixelEngine* Engine;
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

void SceneManager::Release()
{
	nowScene = nullptr;
	for (auto K : SceneMap)
	{
		K.second->Release();
	}
	SceneMap.clear();
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
		PixelLog::Error("Not Find Scene :" + name);
	}
}

GameObject** SceneManager::GetAllSceneObjects(int* outCount)
{
	if (nowScene.IsValid())
	{
		auto block = nowScene.Lock();
		Scene* targetScene = block.GetPtr();
		return targetScene->GetAllSceneObjects(outCount);
	}
	else
	{
		PixelLog::Error("Not Select Scene");
		return 0;
	}
}

void SceneManager::DeleteGameObject(size_t targetObject)
{
	if (nowScene.IsValid() == false)
	{
		auto block = nowScene.Lock();
		block->DeleteGameObject(targetObject);
	}
}

void SceneManager::SaveScene()
{
	if (nowScene.IsValid())
	{
		
	}
}

Scene* SceneManager::GetNowScene()
{
	if (nowScene.IsValid() == false)
	{
		CreateScene("DefaultScene");
		ChangeScene("DefaultScene");
	}
	return nowScene.Lock().GetPtr();
}

void SceneManager::Clear()
{
	nowScene = nullptr;
	for (auto K : SceneMap)
	{
		K.second->Release();
	}
	SceneMap.clear();
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
		PixelLog::Error("This scene name is already in use :" + stem);
	}
}

void SceneManager::RegisterGameObject(SPointer<GameObject> newObject)
{
	if (nowScene.IsValid() == false)
	{
		CreateScene("DefaultScene");
		ChangeScene("DefaultScene");
	}
	auto block = nowScene.Lock();
	block->CreateGameObject(newObject);
}

GameObject* SceneManager::FindGameObject(const std::string& name)
{
	if (nowScene.IsValid() == true)
	{
		auto block = nowScene.Lock();
		return block->FindGameObject(name);
	}

	return nullptr;
}
