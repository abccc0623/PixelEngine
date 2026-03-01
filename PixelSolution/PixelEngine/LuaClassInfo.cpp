#include "pch.h"
#include "LuaClassInfo.h"
#include "PixelEngineAPI.h"
#include "Log.h"
LuaClassInfo::LuaClassInfo(sol::table table, sol::table mtTable)
{
	this->tabel = table;
	this->mataTable = mtTable;
}
LuaClassInfo::~LuaClassInfo()
{

}

void LuaClassInfo::Reload()
{
	
}

void LuaClassInfo::Awake()
{
	sol::state_view lua(this->tabel.lua_state());
	instance = lua.create_table();
	instance[sol::metatable_key] = mataTable;

	luaAwake = instance["Awake"];
	luaStart = instance["Start"];
	luaUpdate = instance["Update"];

	if (luaAwake.valid())
	{
		auto result = luaAwake(instance);
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			Log::Error("--- LUA AWAKE ERROR ---");
			Log::Error(what);
			Log::Error("-----------------------");
		}
	}
}

void LuaClassInfo::Start()
{
	if (luaStart.valid())
	{
		auto result = luaStart(instance);
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			Log::Error("--- LUA SRART ERROR ---");
			Log::Error(what);
			Log::Error("-----------------------");
		}
	}
}

void LuaClassInfo::Update()
{
	if (luaUpdate.valid())
	{
	    auto result = luaUpdate(instance, GetDeltaTime());
	}
}

