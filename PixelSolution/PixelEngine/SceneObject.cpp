#include "pch.h"
#include "SceneObject.h"
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
#include "PoolManager.h"
#include "EventManager.h"

extern PixelEngine* Engine;
extern SceneChangeCallbackFunc g_SceneObjectChangeCallBack;
SceneObject::SceneObject()
{
	sceneName = "";
	path = "";
	info = nullptr;
}
SceneObject::~SceneObject() {}
void SceneObject::Initialize(const std::string& luaPath, const std::string& name)
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
	pool = new ECS::PoolManager();
	event = new ECS::EventManager();
}


void SceneObject::Start()
{
	if (info != nullptr)
	{
		info->Start();
	}
	system->Initialize();
	group->Initialize();
	pool->Initialize();
	event->Initialize();
}

void SceneObject::Update()
{
	if (info != nullptr)
	{
		info->Update();
	}
	system->Update(registry);
}

void SceneObject::Release()
{
	if (info != nullptr)
	{
		info->Release();
	}
	system->Release();
	group->Release();
	pool->Release();
	event->Release();

	delete registry;
	delete system;
	delete group;
	delete pool;
	delete this;
}

uint32_t SceneObject::CreateEntity(const std::string& scriptName)
{
	PixelLog::Info("[" + sceneName + "] CreateEntity :" + scriptName);
	ECS::ChunkedID id = Chunked.Add();
	ECS::EntityObject* entity = Chunked.Get(id);
	entity->Create(scriptName, id.value);
	return id.value;
}

uint32_t SceneObject::CreateGroupAndEntity(const std::string& groupName, const std::string& scriptName)
{
	auto id = CreateEntity(scriptName);
	group->Set(groupName, id);
	return id;
}

uint32_t SceneObject::CreatePoolEntity(const std::string& scriptName)
{
	auto id = CreateEntity(scriptName);
	pool->Register(scriptName, id);
	pool->Disable(scriptName, id);
	return id;
}

ECS::EntityObject* SceneObject::FindEntity(uint32_t id)
{
	ECS::EntityObject* entity = Chunked.Get(id);
	if (entity != nullptr)
	{
		return entity;
	}
	PixelLog::Error("Not Find Entity" + std::to_string(id));
	return nullptr;
}

void SceneObject::ActiveEntity(uint32_t id, bool active)
{
	ECS::EntityObject* entity = Chunked.Get(id);
	if (entity == nullptr)
	{
		PixelLog::Error("Not Find Entity" + std::to_string(id));
		return;
	}
	registry->SetActive(id, active);
}

void SceneObject::DestroyEntity(uint32_t id)
{
	PixelLog::Info("[" + sceneName + "] DeleteEntity");
	auto lua = Engine->GetFactory<LuaManager>();
	if (lua != nullptr)
	{
		lua->RemoveEntityID(id);
	}
	group->RemoveFromAll(id);
	registry->Remove(id);
	Chunked.Remove(ECS::ChunkedID(id));
}

ECS::Registry* SceneObject::GetRegistry()
{
	return registry;
}

const std::string& SceneObject::GetSceneName()
{
	return sceneName;
}


ECS::PoolManager* SceneObject::GetPoolManager()
{
	return pool;
}
ECS::EventManager* SceneObject::GetEventManager()
{
	return event;
}
ECS::GroupManager* SceneObject::GetGroupManager()
{
	return group;
}
