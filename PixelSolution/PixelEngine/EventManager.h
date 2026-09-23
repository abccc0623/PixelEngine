#pragma once
#include "EngineManager.h"
#include "Event.h"
#include <unordered_map>
#include <vector>
#include <sol/forward.hpp>

struct KeyEvent
{
	int keyCode;
	bool Pressed;
};

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

struct CustomDelayEvent
{
	double eventEndTime;
	double eventStartTime;
	std::string eventType;
	sol::table eventTable;
};

struct UserEvent
{
	unsigned int entityID;
	std::string functionName;
};

enum class EventType : int;

namespace ECS
{
	class EventManager
	{
	public:
		EventManager();
		virtual ~EventManager();

		void Initialize();
		void Release();

		bool BindLuaEvent(unsigned int id, const std::string& key, const std::string& func);
		bool CallLuaEvent(const std::string& eventName);
		bool CallLuaEvent(const std::string& eventName, sol::object event);
	private:
		std::vector<CustomDelayEvent> customDelayEvents;
		std::unordered_map<std::string, std::vector<UserEvent>> eventList;
	};
}



