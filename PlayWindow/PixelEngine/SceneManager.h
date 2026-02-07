#pragma once
#include "EngineManager.h"
#include <unordered_map>
#include <string>
class Scene;
class SceneManager : public EngineManager
{
public:
	SceneManager();
	~SceneManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	void ChangeScene(std::string name);
	void CreateScene(std::string name);
private:
	Scene* targetScene;
	std::unordered_map<std::string, Scene*> sceneMap;
};

