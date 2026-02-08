#pragma once
#include <string>
#include <windows.h>
#include <vector>
#include <sol/forward.hpp>
typedef unsigned char byte;
using ObjectID = size_t;
enum RESOURCE_TYPE;
class EngineManager; 
class KeyInputManager;
class TimeManager;
class FactoryManager;
class LuaManager;
class FunctionManager;
class SceneManager;
class GameObject;
class Module;
class Scene;
class TextureManager;
class ResourceManager;
class PixelEngine
{
public:
	PixelEngine() = default;
	~PixelEngine() = default;
	void Initialize(HWND hWnd, int width, int height);
	void Update();
	void Release();
private:
	static constexpr int managerArraySize = 5;
	std::vector<EngineManager*> ManagerList;

	//EngineManager** ManagerList = nullptr;

	TimeManager* timeManager = nullptr;
	KeyInputManager* keyInputManager = nullptr;
	FactoryManager* factoryManager = nullptr;
	LuaManager* luaManager = nullptr;
	FunctionManager* functionManager = nullptr;
	SceneManager* sceneManager = nullptr;
	ResourceManager* resourceManager = nullptr;
public:
	void QuitWindow();
	void ClientUpdate();
	void EngineUpdate();
	bool RunningCheck();

	bool GetKeyDown(byte number);
	bool GetKeyUp(byte number);
	bool GetKey(byte number);
	int GetMousePosition_X();
	int GetMousePosition_Y();

	float GetDeltaTime();
	double GetTotalTime();
	int GetFPS();
	sol::state* GetLua();
	GameObject* CreateGameObject();
	Scene* CreateScene(std::string name);

	ObjectID Load(RESOURCE_TYPE type,const std::string& path);
	ObjectID GetResourceID(RESOURCE_TYPE type,const std::string& path);

	bool LoadLuaScript(const std::string& path);
	bool CreateLuaAPIPath(const std::string& path);
	void RegisterFunction(GameObject* obj, Module* module, int type);
private:
};

