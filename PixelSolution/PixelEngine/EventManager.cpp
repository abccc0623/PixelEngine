#include "pch.h"
#include "EventManager.h"
#include <algorithm>
#include "PixelEngineAPI.h"
#include "Entity.h"

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

void ECS::EventManager::BindLuaEvent(unsigned int id, std::string key, std::string func)
{
	if (eventList.find(key) == eventList.end())
	{
		eventList.insert({ key,std::vector<UserEvent>() });
	}
	UserEvent event;
	event.entityID = id;
	event.functionName = func;
	eventList[key].push_back(event);
}

void ECS::EventManager::CallLuaEvent(std::string eventName, sol::object event)
{
	if (eventList.find(eventName) != eventList.end())
	{
		for (auto& K : eventList[eventName])
		{
			auto entity = FindEntity(K.entityID);
			if (entity != nullptr)
			{
				entity->OnEvent(K.functionName, event);
			}
		}
	}
	else
	{

	}
}
