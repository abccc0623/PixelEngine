#pragma once
#include "Core/Module.h"
class GameObject;
class LuaClassInfo;
class LuaManager;
class LuaScript : public Module
{
public:
	LuaScript();
	virtual ~LuaScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	PIXEL_ENGINEDLL void Reload();
	PIXEL_ENGINEDLL void Register(std::string fileName);
private:
	std::string luaFileName;
	LuaClassInfo* info;
	static LuaManager* lua;
};

