#include "pch.h"
#include "PixelEngineAPI.h"
#include "EventBus.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "EventManager.h"

extern PixelEngine* Engine;
void ECS::EventBus::BindEvent(unsigned int id, const char* eventKey, const char* eventName)
{
	std::string key(eventKey);
	std::string name(eventName);
	auto eventManager = Engine->GetFactory<EventManager>();
	eventManager->BindLuaEvent(id, key, name);
}

void ECS::EventBus::CallEvent(const char* eventKey, sol::object luaTableObj)
{
	std::string key(eventKey);

	if (!luaTableObj.valid() || luaTableObj == sol::nil)
	{
		luaTableObj = sol::make_object(luaTableObj.lua_state(), sol::table::create(luaTableObj.lua_state()));
	}
	auto eventManager = Engine->GetFactory<EventManager>();
	eventManager->CallLuaEvent(key, luaTableObj);
}
