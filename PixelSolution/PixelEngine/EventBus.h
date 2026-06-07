#pragma once
#include <sol/forward.hpp>
namespace ECS::EventBus
{
	void BindEvent(unsigned int id, const char* eventKey, const char* eventName);
	void CallEvent(const char* eventKey, sol::object luaTableObj);
}