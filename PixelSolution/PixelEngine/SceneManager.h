#pragma once
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
	void ReleaseShared() override;

	void RegisterGameObject(SPointer<GameObject> newObject);

	void CreateScene(const std::string& luaPath);
	void ChangeScene(std::string SceneName);
	void SaveScene();
private:
	WPointer<Scene> nowScene = WPointer<Scene>(nullptr);
	std::unordered_map<size_t, SPointer<GameObject>> ObjectList;
	std::unordered_map<std::string, SPointer<Scene>> SceneMap;
};

