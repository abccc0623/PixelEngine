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

	void Reload();
	void Register(std::string fileName);
private:
	LuaClassInfo* info;
	static LuaManager* lua;
};

