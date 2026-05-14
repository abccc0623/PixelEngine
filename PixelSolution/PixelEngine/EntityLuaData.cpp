#include "pch.h"
#include "EntityLuaData.h"
#include "LuaManager.h"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "LuaManager.h"
#include "LuaModuleInfo.h"
#include "Entity.h"
extern PixelEngine* Engine;
LuaManager* ECS::EntityLuaData::lua = nullptr;
static_assert(sizeof(ECS::EntityLuaData) <= 256, "luaDataBuffer is too small!");
ECS::EntityLuaData::EntityLuaData(Entity* target, std::string scriptName)
{
	this->scriptName = scriptName;
	if (lua == nullptr)
	{
		lua = Engine->GetFactory<LuaManager>();
	}
	auto luaInfo = lua->GetModuleLua(scriptName);
	if (luaInfo == nullptr)
	{
		PixelLog::Error("Not Find Lua File :" + scriptName);
		return;
	}
	instance = luaInfo->Create();
	spawn = instance["Spawn"];

	entityID = target->GetID();
	instance["entity"] = target;
	instance["ID"] = entityID;
	lua->AddEntityID(entityID, instance);
}

ECS::EntityLuaData::~EntityLuaData()
{
	if (instance.valid())
	{
		instance["entity"] = sol::lua_nil;
		lua->RemoveEntityID(entityID);
	}
}

void ECS::EntityLuaData::SpawnCall()
{
	if (spawn.valid())
	{
		auto result = spawn(instance);
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			PixelLog::Error("--- LUA SPAWN ERROR ---");
			PixelLog::Error(what);
			PixelLog::Error("-----------------------");
		}
	}
}


