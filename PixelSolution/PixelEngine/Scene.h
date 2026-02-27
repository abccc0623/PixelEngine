#pragma once
#include "PixelObject.h"
#include "sol.hpp"
#include <unordered_map>
struct RenderingData;
class GameObject;
class LuaManager;
class FunctionManager;
class Scene : public PixelObject
{
public:
	Scene();
	~Scene();
	void Initialize(const std::string& luaPath,const std::string& name);
	void Start();
	void Update();
	void Release();
	void CreateGameObject(SPointer<GameObject>& Obj);


	std::string Save(int tab);
private:
	std::string sceneName;
	std::string path;
	LuaManager* lua;
	FunctionManager* func;

	sol::table table;
	sol::protected_function luaStart;
	sol::protected_function luaRelease;

	sol::protected_function OnStartFunc;
	sol::protected_function OnReleaseFunc;

	std::unordered_map<size_t, SPointer<GameObject>> ObjectList;
};

