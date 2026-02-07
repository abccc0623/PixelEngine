#pragma once
#include "EngineManager.h";
#include <string>
struct lua_State;
class LuaManager : public EngineManager
{
public:
	LuaManager();
	~LuaManager();

	// EngineManager을(를) 통해 상속됨
	void Initialize() override;
	void Update() override;
	void Release() override;
	bool LoadLuaScript(const std::string& fileName);

private:
	lua_State* luaState;
};

