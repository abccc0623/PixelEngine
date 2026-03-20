#pragma once
#include <sol/forward.hpp>
class LuaSceneInfo
{
public:
	LuaSceneInfo(sol::table table);
	~LuaSceneInfo();

	void Start();
	void Update();
	void Release();
private:
	sol::table table;
};

