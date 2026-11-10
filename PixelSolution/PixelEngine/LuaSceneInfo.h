#pragma once
#include <sol/forward.hpp>
#include <memory>
class LuaSceneInfo
{
public:
	LuaSceneInfo(sol::table table);
	LuaSceneInfo(sol::protected_function factory);
	std::unique_ptr<LuaSceneInfo> Create();
	~LuaSceneInfo();

	void Start();
	void Update();
	void Release();
private:
	sol::table table;
	sol::protected_function factory;
};

