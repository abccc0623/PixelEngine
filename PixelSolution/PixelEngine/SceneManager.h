#pragma once
#include "PixelEngineAPI.h"
#include "EngineManager.h"
#include "SPointer.h"
#include "WPointer.h"
#include <unordered_map>
#include <string>
class SceneObject;
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
	ECS::EntityObject* FindEntity(uint32_t id);
	void ActiveEntity(uint32_t id, bool active);
	void DestroyEntity(uint32_t id);
	ECS::Registry* GetRegistry();

	void CreateScene(const std::string& luaPath);
	void ChangeScene(std::string SceneName);
	SceneObject* GetNowScene();
private:
	SceneObject* nowScene = nullptr;
	std::unordered_map<std::string, SceneObject*> SceneMap;
};

