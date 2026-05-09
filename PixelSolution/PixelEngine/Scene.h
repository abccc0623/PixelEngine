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

	void CreateGameObject(SPointer<GameObject> Obj);
	void DeleteGameObject(size_t targetObject);
	ECS::Registry* GetRegistry();


	GameObject** GetAllSceneObjects(int* maxCount);
	GameObject* FindGameObject(const std::string& name);
	WPointer<GameObject> FindGameObjectToEngine(const std::string& name);
	WPointer<GameObject> FindGameObjectToEngine(GameObject* target);

	std::string sceneName;
private:
	std::string path;
	std::unordered_map<size_t, SPointer<GameObject>> ObjectList;
	std::vector<GameObject*> Getter;
	int ObjectCount = 0;
	LuaSceneInfo* info;
private:
	LuaManager* lua;
	FunctionManager* func;

	ECS::EntityArray* entityArray;
	ECS::Registry* registry;
};

