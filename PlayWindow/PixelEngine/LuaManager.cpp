#include "LuaManager.h"
#include "LuaWrapper.h"



LuaManager::LuaManager()
{

}

LuaManager::~LuaManager()
{

}

void LuaManager::Initialize()
{
	luaState = lua_open();
	luaL_openlibs(luaState);

    LuaRegister(luaState);
}


bool LuaManager::LoadLuaScript(const std::string& fileName)
{
    // luaL_dofile은 내부적으로 luaL_loadfile과 lua_pcall을 합친 매크로입니다.
    if (luaL_dofile(luaState, fileName.c_str()) != 0)
    {
        const char* error = lua_tostring(luaState, -1);
        printf("Lua Load Error: %s\n", error);
        lua_pop(luaState, 1); 
		return false;
    }
	return true;
}




void LuaManager::Update()
{

}

void LuaManager::Release()
{

}
