#pragma once
#include <sol/forward.hpp>
namespace ECS::LuaEvent
{
	struct LuaEventData
	{
		float test;
	};

	void* AddComponent(unsigned int id);
	void* GetComponent(unsigned int id);
	bool HasComponent(unsigned int id);

	void BindEvent(unsigned int id, const char* eventKey, const char* eventName);
	void CallEvent(const char* eventKey, sol::object luaTableObj);
	std::string BindJit();
}