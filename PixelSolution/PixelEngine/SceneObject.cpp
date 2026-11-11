#include "pch.h"
#include "SceneObject.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "sol.hpp"
#include "Log.h"
#include "Export/PixelEngineAPI.h"
#include "LuaSceneInfo.h"

#include "Registry.h"
#include "SystemManager.h"
#include "GroupManager.h"
#include "PoolManager.h"
#include "EventManager.h"
#include "TweenManager.h"
#include "TimeAPI.h"
#include "PhysManager.h"
#include "Physics2DData.h"

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
	registry = new ECS::Registry();
	system = new ECS::SystemManager();
	group = new ECS::GroupManager();
	pool = new ECS::PoolManager();
	event = new ECS::EventManager();
	tween = new ECS::TweenManager();
	if (!luaPath.empty())
	{
		auto definition = Engine->GetFactory<LuaManager>()->GetSceneLua(name);
		if (definition != nullptr) info = definition->Create();
	}
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
	if (IsPlayMode())
	{
		tween->Update(registry,Time_GetDeltaTime(),Time_GetUnscaledDeltaTime());
	}
	system->Update(registry);
}

void SceneObject::Release()
{
	if (registry == nullptr) return;
	if (info != nullptr) info->Release();

	// Entity storage remains occupied when its components are disabled by a pool.
	// Snapshot before recursive deletion so inactive entities are included as well.
	std::vector<uint32_t> entities;
	Chunked.ForEach([&entities](ECS::EntityObject* entity, size_t)
	{
		entities.push_back(entity->GetID());
	});
	std::unordered_set<uint32_t> destroyingIDs;
	for (uint32_t id : entities) DestroyEntityHierarchy(id, destroyingIDs);

	system->Release();
	group->Release();
	pool->Release();
	event->Release();
	tween->Clear();
	delete registry;
	delete system;
	delete group;
	delete pool;
	delete event;
	delete tween;
	registry = nullptr;
	system = nullptr;
	group = nullptr;
	pool = nullptr;
	event = nullptr;
	tween = nullptr;
	info.reset();
}

uint32_t SceneObject::CreateEntity(const std::string& scriptName)
{
	PixelLog::Info("[" + sceneName + "] CreateEntity :" + scriptName);
	ECS::ChunkedID id = Chunked.Add();
	ECS::EntityObject* entity = Chunked.Get(id);
	// Chunk 등록에 실패한 상태에서 EntityObject를 역참조하면 프로세스가 종료된다.
	if (entity == nullptr)
	{
		PixelLog::Error("[SceneObject][CreateEntity] Invalid entity slot. ID: " + std::to_string(id.value));
		return UINT32_MAX;
	}
	entity->Create(scriptName, id.value);
	return id.value;
}

uint32_t SceneObject::CreateGroupAndEntity(const std::string& groupName, const std::string& scriptName)
{
	auto id = CreateEntity(scriptName);
	if (id == ECS::ChunkedID::InvalidValue) return id;
	group->Set(groupName, id);
	return id;
}

uint32_t SceneObject::CreatePoolEntity(const std::string& scriptName)
{
	auto id = CreateEntity(scriptName);
	if (id == ECS::ChunkedID::InvalidValue) return id;
	pool->Register(scriptName, id);
	pool->Disable(scriptName, id);
	return id;
}

ECS::EntityObject* SceneObject::FindEntity(uint32_t id)
{
	ECS::EntityObject* entity = TryFindEntity(id);
	if (entity != nullptr)
	{
		return entity;
	}
	PixelLog::Error("[SceneObject][FindEntity] Invalid or stale Entity ID: " + std::to_string(id));
	return nullptr;
}

ECS::EntityObject* SceneObject::TryFindEntity(uint32_t id)
{
	return Chunked.Get(id);
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

bool SceneObject::CanDestroyEntity(uint32_t id)
{
	if (Chunked.Get(ECS::ChunkedID(id)) == nullptr)
	{
		PixelLog::Error("[SceneObject][DestroyEntity] Invalid or stale Entity ID: " + std::to_string(id));
		return false;
	}

	// 계층을 일부 삭제한 뒤 거부하지 않도록 전체를 먼저 검사한다.
	std::vector<uint32_t> pending{ id };
	std::unordered_set<uint32_t> visited;
	while (!pending.empty())
	{
		const uint32_t currentID = pending.back();
		pending.pop_back();
		if (!visited.insert(currentID).second) continue;

		auto entity = Chunked.Get(ECS::ChunkedID(currentID));
		if (entity == nullptr) continue;
		if (!entity->IsSpawnComplete())
		{
			PixelLog::Warn("[SceneObject][DestroyEntity] Ignored while Spawn is in progress. Requested ID: " +
				std::to_string(id) + ", Initializing ID: " + std::to_string(currentID));
			return false;
		}

		const auto& children = entity->GetChild();
		pending.insert(pending.end(), children.begin(), children.end());
	}
	return true;
}

void SceneObject::DestroyEntity(uint32_t id)
{
	if (!CanDestroyEntity(id)) return;

	std::unordered_set<uint32_t> destroyingIDs;
	DestroyEntityHierarchy(id, destroyingIDs);
}

void SceneObject::DestroyEntityHierarchy(uint32_t id, std::unordered_set<uint32_t>& destroyingIDs)
{
	if (!destroyingIDs.insert(id).second)
	{
		return;
	}

	ECS::EntityObject* entity = Chunked.Get(ECS::ChunkedID(id));
	if (entity == nullptr)
	{
		return;
	}

	const uint32_t parentID = entity->GetParentID();
	if (parentID != UINT32_MAX)
	{
		ECS::EntityObject* parent = Chunked.Get(ECS::ChunkedID(parentID));
		if (parent != nullptr)
		{
			auto& parentChildren = parent->GetChild();
			parentChildren.erase(
				std::remove(parentChildren.begin(), parentChildren.end(), id),
				parentChildren.end());
		}
	}

	const std::vector<unsigned int> children = entity->GetChild();
	for (const unsigned int childID : children)
	{
		DestroyEntityHierarchy(childID, destroyingIDs);
	}

	PixelLog::Info("[" + sceneName + "] DeleteEntity");
	auto lua = Engine->GetFactory<LuaManager>();
	if (lua != nullptr)
	{
		lua->RemoveEntityID(id);
	}
	group->RemoveFromAll(id);
	tween->RemoveFromAll(id);
	Engine->GetFactory<PhysManager>()->DestroyRigidbody(registry->Get<Physics2DData>(id));
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

ECS::TweenManager* SceneObject::GetTweenManager()
{
	return tween;
}
