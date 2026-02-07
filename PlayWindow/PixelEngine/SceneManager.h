#pragma once
#include "EngineManager.h"
class Scene;
class SceneManager : public EngineManager
{
public:
	SceneManager();
	~SceneManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	void ChangeScene(Scene* scene);
private:
	Scene* targetScene;
};

