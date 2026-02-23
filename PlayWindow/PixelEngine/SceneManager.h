#pragma once
#include "EngineManager.h"
#include "SPointer.h"
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

	void ChangeScene(std::string name);
	void CreateScene(const std::string& name);

	void Register(SPointer<GameObject> newObject);
private:
	Scene* targetScene;
	std::unordered_map<std::string, Scene*> sceneMap;



	std::unordered_map<size_t, SPointer<GameObject>> ObjectList;
};

