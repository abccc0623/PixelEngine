#pragma once
#include "PixelObject.h"
#include <string>
#include <windows.h>
#include <vector>
#include <sol/forward.hpp>
#include <typeindex>
#include <unordered_map>
typedef unsigned char byte;
using ObjectID = size_t;
enum RESOURCE_TYPE;
class EngineManager; 
class KeyInputManager;
class TimeManager;
class ObjectManager;
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
	void ReleaseShared();
	void Resize(int width, int height);
public:
	void QuitWindow();
	void ClientUpdate();
	void EngineUpdate();
	bool RunningCheck();

	sol::state* GetModuleCall_Lua();
	SPointer<GameObject>& CreateGameObject(std::string name = "GameObject");
	Scene* CreateScene(std::string name);

	ObjectID GetResourceID(RESOURCE_TYPE type,const std::string& path);

	void AddFunction(GameObject* obj, Module* module, int type);

	template<std::derived_from<EngineManager> T>
	void BindFactory()
	{
		std::type_index key = typeid(T);
		factoryMap.insert({ key ,new T()});
	};

	template<std::derived_from<EngineManager> T>
	T* GetFactory()
	{
		std::type_index key = typeid(T);
		auto target = factoryMap.find(key);
		if (target != factoryMap.end())
		{
			return static_cast<T*>(target->second);
		}
		return nullptr;
	}
private:
	std::unordered_map<std::type_index, EngineManager*> factoryMap;
};

