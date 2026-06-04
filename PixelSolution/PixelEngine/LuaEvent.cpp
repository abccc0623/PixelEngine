#include "pch.h"
#include "PixelEngineAPI.h"
#include "LuaEvent.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "EventManager.h"

extern PixelEngine* Engine;
void* ECS::LuaEvent::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<LuaEventData>(id);
	auto data = registry->Get<LuaEventData>(id);
	return data;
}

void* ECS::LuaEvent::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	LuaEventData* data = registry->Get<LuaEventData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[LuaEvent][GetComponent] Not Find Component");
	}
	return data;
}

bool ECS::LuaEvent::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	LuaEventData* data = registry->Get<LuaEventData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ECS::LuaEvent::BindEvent(unsigned int id, const char* eventKey, const char* eventName)
{
	std::string key(eventKey);
	std::string name(eventName);

	auto eventManager = Engine->GetFactory<EventManager>();
	eventManager->BindLuaEvent(id, key, name);
}

void ECS::LuaEvent::CallEvent(const char* eventKey, sol::object luaTableObj)
{
	std::string key(eventKey);

	if (!luaTableObj.valid() || luaTableObj == sol::nil)
	{
		luaTableObj = sol::make_object(luaTableObj.lua_state(), sol::table::create(luaTableObj.lua_state()));
	}

	auto eventManager = Engine->GetFactory<EventManager>();
	eventManager->CallLuaEvent(key, luaTableObj);
}


std::string ECS::LuaEvent::BindJit()
{
	std::string jit = R"(
	---@class LuaEventData
ffi.cdef[[
    typedef struct 
    { 
        float test;
    } LuaEventData;
]]
    )";
	return jit;
}
