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



enum class EventType : int;
class GameObject;
class EventManager : public  EngineManager
{
public:
	EventManager();
	virtual ~EventManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void RegisterMessageCustom(GameObject* target, std::string EventName);
	void UnregisterMessageCustom(GameObject* target, std::string EventName);

	void RegisterMessage(GameObject* target, EventType type);
	void UnregisterMessage(GameObject* target, EventType type);

	void TriggerCustomEvent(std::string eventType, sol::table event, float time);
	void TriggerEvent(EventType type, Event event);
private:
	std::unordered_map<EventType, std::vector<GameObject*>> eventList;
	std::unordered_map<std::string, std::vector<GameObject*>> customEventList;
	std::vector<CustomDelayEvent> customDelayEvents;
};

