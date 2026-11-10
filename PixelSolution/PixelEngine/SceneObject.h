#pragma once
#include "sol.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include "ChunkedArray.h"
#include "EntityObject.h"

struct RenderingData;
class GameObject;
class LuaManager;
class FunctionManager;
class SceneManager;
class LuaSceneInfo;

namespace ECS
{
	class Registry;
	class SystemManager;
	class GroupManager;
	class PoolManager;
	class EventManager;
	class TweenManager;
}
class SceneObject
{
public:
	SceneObject();
	~SceneObject();
	void Initialize(const std::string& luaPath, const std::string& name);
	void Start();
	void Update();
	void Release();

	uint32_t CreateEntity(const std::string& scriptName);
	uint32_t CreateGroupAndEntity(const std::string& groupName, const std::string& scriptName);
	uint32_t CreatePoolEntity(const std::string& scriptName);


	ECS::EntityObject* FindEntity(uint32_t id);
	void ActiveEntity(uint32_t id, bool active);

	bool CanDestroyEntity(uint32_t id);
	void DestroyEntity(uint32_t id);
	ECS::Registry* GetRegistry();

	const std::string& GetSceneName();
	ECS::GroupManager* GetGroupManager();
	ECS::PoolManager* GetPoolManager();
	ECS::EventManager* GetEventManager();
	ECS::TweenManager* GetTweenManager();
private:
	void DestroyEntityHierarchy(uint32_t id, std::unordered_set<uint32_t>& destroyingIDs);

	std::string sceneName;
	std::string path;
	std::unique_ptr<LuaSceneInfo> info;
private:
	ECS::ChunkedArray<ECS::EntityObject, 100> Chunked;
	ECS::Registry* registry = nullptr;
	ECS::SystemManager* system = nullptr;
	ECS::GroupManager* group = nullptr;
	ECS::PoolManager* pool = nullptr;
	ECS::EventManager* event = nullptr;
	ECS::TweenManager* tween = nullptr;
};

