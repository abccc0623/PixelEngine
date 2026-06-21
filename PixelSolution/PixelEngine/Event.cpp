#include "pch.h"
#include "Event.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "EventManager.h"
#include "Scene.h"
#include "SceneManager.h"
extern PixelEngine* Engine;
void ECS::Event::BindEvent(unsigned int id, const char* eventKey, const char* eventName)
{
	std::string key(eventKey);
	std::string name(eventName);

	auto sceneManager = Engine->GetFactory<SceneManager>();
	sceneManager->GetNowScene()->GetEventManager()->BindLuaEvent(id, key, name);
}


void ECS::Event::CallEvent(const char* eventKey, sol::object luaTableObj)
{
	std::string key(eventKey);

	if (!luaTableObj.valid() || luaTableObj == sol::nil)
	{
		luaTableObj = sol::make_object(luaTableObj.lua_state(), sol::table::create(luaTableObj.lua_state()));
	}
	auto sceneManager = Engine->GetFactory<SceneManager>();
	sceneManager->GetNowScene()->GetEventManager()->CallLuaEvent(key, luaTableObj);
}
