#pragma once
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "luaconf.h"
#include "lualib.h"
}
#include "PixelEngineAPI.h"

static int Lua_GetKey(lua_State* state);
static int Lua_GetKeyDown(lua_State* state);
static int Lua_GetKeyUp(lua_State* state);
static int Lua_GetFPS(lua_State* state);

static void LuaRegister(lua_State* L)
{
	lua_register(L, "GetKey", &Lua_GetKey);
	lua_register(L, "GetKeyDown", &Lua_GetKeyDown);
	lua_register(L, "GetKeyUp", &Lua_GetKeyUp);
	lua_register(L, "GetFPS", &Lua_GetFPS);
}


static int Lua_GetKey(lua_State* state)
{
	bool success = false;
	if (lua_isstring(state, 1)) {
		const char* title = lua_tostring(state, 1);
		lua_pushboolean(state, GetKey(title[0]));
	}
	return 1;
}
static int Lua_GetKeyDown(lua_State* state)
{
	bool success = false;
	if (lua_isstring(state, 1)) {
		const char* title = lua_tostring(state, 1);
		lua_pushboolean(state, GetKeyDown(title[0]));
	}
	return 1;
}
static int Lua_GetKeyUp(lua_State* state)
{
	bool success = false;
	if (lua_isstring(state, 1)) {
		const char* title = lua_tostring(state, 1);
		lua_pushboolean(state, GetKeyUp(title[0]));
	}
	return 1;
}
static int Lua_GetFPS(lua_State* state)
{
	if (lua_isstring(state, 1)) {
		lua_pushnumber(state, GetFPS());
	}
	return 1;
}