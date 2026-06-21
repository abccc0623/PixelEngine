#pragma once
#include "EngineManager.h"
#include "Event.h"
#include <unordered_map>
#include <vector>
#include <sol/forward.hpp>

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

		void BindLuaEvent(unsigned int id, std::string key, std::string func);
		void CallLuaEvent(std::string eventName, sol::object event);
	private:
		std::vector<CustomDelayEvent> customDelayEvents;
		std::unordered_map<std::string, std::vector<UserEvent>> eventList;
	};
}



