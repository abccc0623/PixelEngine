#include "pch.h"
#include "EventManager.h"
#include <algorithm>
#include "PixelEngineAPI.h"
#include "EntityObject.h"

ECS::EventManager::EventManager()
{

}

ECS::EventManager::~EventManager()
{

}

void ECS::EventManager::Initialize()
{

}

void ECS::EventManager::Release()
{
	eventList.clear();
}

bool ECS::EventManager::BindLuaEvent(unsigned int id, const std::string& key, const std::string& func)
{
	if (key.empty() || func.empty() || FindEntity(id) == nullptr)
	{
		return false;
	}

	auto& events = eventList[key];
	auto duplicate = std::find_if(events.begin(), events.end(), [id, &func](const UserEvent& event)
		{
			return event.entityID == id && event.functionName == func;
		});
	if (duplicate == events.end())
	{
		events.push_back({ id, func });
	}
	return true;
}

bool ECS::EventManager::CallLuaEvent(const std::string& eventName)
{
	auto found = eventList.find(eventName);
	if (found == eventList.end()) return false;

	bool called = false;
	const auto events = found->second;
	for (const auto& event : events)
	{
		auto entity = FindEntity(event.entityID);
		if (entity != nullptr)
		{
			entity->OnEvent(event.functionName);
			called = true;
		}
	}
	return called;
}

bool ECS::EventManager::CallLuaEvent(const std::string& eventName, sol::object eventData)

{
	auto found = eventList.find(eventName);
	if (found == eventList.end()) return false;

	bool called = false;
	const auto events = found->second;
	for (const auto& event : events)
	{
		auto entity = FindEntity(event.entityID);
		if (entity != nullptr)
		{
			entity->OnEvent(event.functionName, eventData);
			called = true;
		}
	}
	return called;
}
