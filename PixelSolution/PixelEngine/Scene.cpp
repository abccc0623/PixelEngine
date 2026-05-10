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

#include "EntityArray.h"
#include "Registry.h"
#include "SystemManager.h"

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
	auto lua = Engine->GetFactory<LuaManager>();
	info = lua->GetSceneLua(name);

	entityArray = new ECS::EntityArray();
	registry = new ECS::Registry();
	system = new ECS::SystemManager();
}


void Scene::Start()
{
	if (info != nullptr)
	{
		info->Start();
		system->Initialize();
	}
}

void Scene::Update()
{
	if (info != nullptr)
	{
		info->Update();
	}
	entityArray->Update();
	system->Update(registry);
}

void Scene::Release()
{
	if (info != nullptr)
	{
		info->Release();
	}
	system->Release();

	delete entityArray;
	delete registry;
	delete system;
}

uint32_t Scene::CreateEntity(const std::string& scriptName)
{
	PixelLog::Info("[" + sceneName + "] CreateEntity :" + scriptName);
	return entityArray->Create(scriptName);
}

void Scene::DestroyEntity(uint32_t id)
{
	PixelLog::Info("[" + sceneName + "] DeleteEntity");
	ECS::EntityID entityID(id);
	entityArray->Destroy(entityID);
}

ECS::Registry* Scene::GetRegistry()
{
	return registry;
}
