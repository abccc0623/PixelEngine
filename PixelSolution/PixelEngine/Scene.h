#pragma once
#include "Core/PixelObject.h"
#include "sol.hpp"
#include <unordered_map>
#include <vector>


template <typename T> class SPointer;
template <typename T> class WPointer;
struct RenderingData;
class GameObject;
class LuaManager;
class FunctionManager;
class SceneManager;
class LuaSceneInfo;

namespace ECS
{
	class EntityArray;
	class Registry;
	class SystemManager;
}
class Scene : public PixelObject
{
public:
	Scene();
	~Scene();
	void Initialize(const std::string& luaPath,const std::string& name);
	void Start();
	void Update();
	void Release();

	uint32_t CreateEntity(const std::string& scriptName);
	void DestroyEntity(uint32_t id);
	ECS::Registry* GetRegistry();

	std::string sceneName;
private:
	std::string path;
	LuaSceneInfo* info;
private:
	ECS::EntityArray* entityArray;
	ECS::Registry* registry;
	ECS::SystemManager* system;
};

