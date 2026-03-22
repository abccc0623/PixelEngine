#pragma once

#include <sol/forward.hpp>
class Transform;
class GameObject;
class LuaModuleInfo
{
public:
	LuaModuleInfo(sol::table table);
	~LuaModuleInfo();

	void Reload();
	void Awake();
	void Start();
	void Update();

	void Set(Transform* transform);
	void Set(GameObject* targetObject);
private:
	sol::table tabel;
	sol::table instance;
	sol::table mataTable;
	sol::protected_function luaAwake;
	sol::protected_function luaStart;
	sol::protected_function luaUpdate;
	Transform* transform;
	GameObject* targetObject;
};

