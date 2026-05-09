#pragma once
#include "PixelEngineAPI.h"
#include "EngineManager.h"
#include "SPointer.h"
#include "WPointer.h"
#include <unordered_map>
#include <string>
class Scene;
class GameObject;
class SceneManager : public EngineManager
{
public:
	SceneManager();
	~SceneManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	uint32_t CreateEntity(const std::string& scriptName);
	void DestroyEntity(uint32_t id);

	void RegisterGameObject(SPointer<GameObject> newObject);
	GameObject* FindGameObject(const std::string& name);

	void CreateScene(const std::string& luaPath);
	void ChangeScene(std::string SceneName);
	GameObject** GetAllSceneObjects(int* outCount);
	void DeleteGameObject(size_t targetObject);
	void SaveScene();
	Scene* GetNowScene();
private:
	WPointer<Scene> nowScene = WPointer<Scene>(nullptr);
	std::unordered_map<std::string, SPointer<Scene>> SceneMap;
};

