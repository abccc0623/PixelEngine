#pragma once


struct KeyEvent
{
	int keyCode;
	bool Pressed;
};

class GameObject;
struct PhysEvent
{
	bool targetIn;
};

struct EventMessage
{
	union
	{
		KeyEvent key;
		PhysEvent Collision;
	};
};
#include <sol/forward.hpp>
namespace ECS::Event
{
	void BindEvent(unsigned int id, const char* eventKey, const char* eventName);
	void CallEvent(const char* eventKey, sol::object luaTableObj);
}