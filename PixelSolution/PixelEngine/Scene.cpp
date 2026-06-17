#include "pch.h"
#include "Scene.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "sol.hpp"
#include "SPointer.h"
#include "WPointer.h"
#include "Log.h"
#include "Export/PixelEngineAPI.h"
#include "LuaSceneInfo.h"

#include "Registry.h"
#include "SystemManager.h"
#include "GroupManager.h"

extern PixelEngine* Engine;
extern SceneChangeCallbackFunc g_SceneObjectChangeCallBack;
Scene::Scene()
{
	sceneName = "";
	path = "";
	info = nullptr;
}
Scene::~Scene() {}
void Scene::Initialize(const std::string& luaPath, const std::string& name)
{
	sceneName = name;
	path = luaPath;
	if (!luaPath.empty())
	{
		auto lua = Engine->GetFactory<LuaManager>();
		info = lua->GetSceneLua(name);
	}

	registry = new ECS::Registry();
	system = new ECS::SystemManager();
	group = new ECS::GroupManager();
}


void Scene::Start()
{
	if (info != nullptr)
	{
		info->Start();
	}
	system->Initialize();
	group->Initialize();
}

void Scene::Update()
{
	if (info != nullptr)
	{
		info->Update();
	}
	system->Update(registry);
}

void Scene::Release()
{
	if (info != nullptr)
	{
		info->Release();
	}
	system->Release();
	group->Release();

	delete registry;
	delete system;
	delete group;
	delete this;
}

uint32_t Scene::CreateEntity(const std::string& scriptName)
{
	PixelLog::Info("[" + sceneName + "] CreateEntity :" + scriptName);
	ECS::ChunkedID id = Chunked.Add();
	ECS::Entity* entity = Chunked.Get(id);
	entity->Create(scriptName, id.value);
	return id.value;
}

uint32_t Scene::CreateGroupEntity(const std::string& groupName, const std::string& scriptName)
{
	auto id = CreateEntity(scriptName);
	group->Set(groupName, id);
	return id;
}

ECS::Entity* Scene::FindEntity(uint32_t id)
{
	ECS::Entity* entity = Chunked.Get(id);
	if (entity != nullptr)
	{
		return entity;
	}
	PixelLog::Error("Not Find Entity" + std::to_string(id));
	return nullptr;
}

void Scene::ActiveEntity(uint32_t id, bool active)
{
	ECS::Entity* entity = Chunked.Get(id);
	if (entity == nullptr)
	{
		PixelLog::Error("Not Find Entity" + std::to_string(id));
		return;
	}
	registry->SetActive(id, active);
	entity->SetActive(active);
}

void Scene::DestroyEntity(uint32_t id)
{
	PixelLog::Info("[" + sceneName + "] DeleteEntity");
	group->RemoveFromAll(id);
	registry->Remove(id);
}

ECS::Registry* Scene::GetRegistry()
{
	return registry;
}

const std::string& Scene::GetSceneName()
{
	return sceneName;
}

ECS::GroupManager* Scene::GetGroupManager()
{
	return group;
}
