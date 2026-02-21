#pragma once
#include "Module.h"
#include <string>
#include "sol.hpp"
class LuaScript : public Module
{
public:
	LuaScript();
	virtual ~LuaScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	void Reload();
	void RegisterFile(std::string fileName);
	static std::string RegisterLua();
private:
	bool updateLog = false;
	std::string path;
	sol::state lua;
	sol::table selfTable;
	sol::protected_function luaAwake;
	sol::protected_function luaStart;
	sol::protected_function luaUpdate;
};

