#pragma once
#include "PixelEngineAPI.h"
#include "EngineManager.h"
#include "SPointer.h"
#include "WPointer.h"
#include <unordered_map>
#include <string>
class Scene;
class GameObject;
namespace ECS
{
	class Registry;
}
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
	ECS::Registry* GetRegistry();

	void CreateScene(const std::string& luaPath);
	void ChangeScene(std::string SceneName);
	Scene* GetNowScene();
private:
	WPointer<Scene> nowScene = WPointer<Scene>(nullptr);
	std::unordered_map<std::string, SPointer<Scene>> SceneMap;
};

