#pragma once
#include "sol.hpp"
#include <unordered_map>
#include <vector>
#include "ChunkedArray.h"
#include "Entity.h"

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
}
class Scene
{
public:
	Scene();
	~Scene();
	void Initialize(const std::string& luaPath, const std::string& name);
	void Start();
	void Update();
	void Release();

	uint32_t CreateEntity(const std::string& scriptName);
	uint32_t CreateGroupEntity(const std::string& groupName, const std::string& scriptName);
	uint32_t CreatePoolEntity(const std::string& poolName, const std::string& scriptName);


	ECS::Entity* FindEntity(uint32_t id);
	void ActiveEntity(uint32_t id, bool active);

	void DestroyEntity(uint32_t id);
	ECS::Registry* GetRegistry();

	const std::string& GetSceneName();
	ECS::GroupManager* GetGroupManager();
	ECS::PoolManager* GetPoolManager();
private:
	std::string sceneName;
	std::string path;
	LuaSceneInfo* info;
private:
	ECS::ChunkedArray<ECS::Entity, 100> Chunked;
	ECS::Registry* registry;
	ECS::SystemManager* system;
	ECS::GroupManager* group;
	ECS::PoolManager* pool;
};

