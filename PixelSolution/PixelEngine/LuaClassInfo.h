#pragma once

#include <sol/forward.hpp>
class LuaClassInfo
{
public:
	LuaClassInfo(sol::table table);
	~LuaClassInfo();

	void Reload();
	void Awake();
	void Start();
	void Update();
private:
	sol::table tabel;
	sol::protected_function luaAwake;
	sol::protected_function luaStart;
	sol::protected_function luaUpdate;
};

