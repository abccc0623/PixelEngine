#pragma once
#include "Module.h"
#include <string>
#include "sol.hpp"
class GameObject;
class LuaScript : public Module
{
public:
	LuaScript();
	virtual ~LuaScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	//void OnCollisionEnter2D(WPointer<GameObject> target) override;
	void OnCollision2D(WPointer<GameObject> target) override;
	//void OnCollisionExit2D(WPointer<GameObject> target) override;

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

	sol::protected_function OnCollision2DFunc;
	sol::protected_function OnCollision2DEnterFunc;
	sol::protected_function OnCollision2DExitFunc;
};

