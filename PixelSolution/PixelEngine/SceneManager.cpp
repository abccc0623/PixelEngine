#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Log.h"
#include "JsonManager.h"
#include "PixelEngine.h"

extern PixelEngine* Engine;
SceneManager::SceneManager()
{
	nowScene = nullptr;
	SceneMap = std::unordered_map<std::string, Scene*>();
}

SceneManager::~SceneManager()
{

}

void SceneManager::Initialize()
{

}

void SceneManager::Update()
{
	if (nowScene != nullptr)
	{
		nowScene->Update();
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
			nowScene->Release();
		}
		//씬 변경
		nowScene = SceneMap[name];

		nowScene->Start();
	}
	else
	{
		PixelLog::Error("Not Find Scene :" + name);
	}
}

Scene* SceneManager::GetNowScene()
{
	if (nowScene == nullptr)
	{
		CreateScene("DefaultScene");
		ChangeScene("DefaultScene");
	}
	return nowScene;
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

uint32_t SceneManager::CreateEntity(const std::string& scriptName)
{
	if (nowScene == nullptr)
	{
		CreateScene("DefaultScene");
		ChangeScene("DefaultScene");
	}
	return nowScene->CreateEntity(scriptName);
}

void SceneManager::DestroyEntity(uint32_t id)
{
	if (nowScene == nullptr)
	{
		CreateScene("DefaultScene");
		ChangeScene("DefaultScene");
	}
	return nowScene->DestroyEntity(id);
}

ECS::Registry* SceneManager::GetRegistry()
{
	auto scene = GetNowScene();
	return scene->GetRegistry();
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
		Scene* newScene = new Scene();
		newScene->Initialize(luaPath, stem);
		SceneMap.insert({ stem,newScene });
	}
	else
	{
		PixelLog::Error("This scene name is already in use :" + stem);
	}
}
