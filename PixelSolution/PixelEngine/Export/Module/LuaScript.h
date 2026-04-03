#pragma once
#include "Core/Module.h"
#include <sol/forward.hpp>
class GameObject;
class LuaModuleInfo;
class LuaManager;
class LuaScript : public Module
{
public:
	LuaScript();
	virtual ~LuaScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	void MessageHub();
	void Reload();
	void Register(std::string fileName);
private:
	std::string luaFileName;
	sol::table instance;
	sol::function awake;
	sol::function start;
	sol::function update;
	sol::function message;

	static LuaManager* lua;
};

