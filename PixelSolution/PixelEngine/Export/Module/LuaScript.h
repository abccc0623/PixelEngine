#pragma once
#include "Core/Module.h"
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

	void Reload();
	void Register(std::string fileName);
private:
	std::string luaFileName;
	LuaModuleInfo* info;
	static LuaManager* lua;
};

