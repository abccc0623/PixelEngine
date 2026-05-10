#include "pch.h"
#include "EventManager.h"
#include <algorithm>
#include "PixelMetaAPI.h"
#include "PixelEngineAPI.h"

EventManager::EventManager()
{

}

EventManager::~EventManager()
{

}

void EventManager::Initialize()
{

}

void EventManager::Update()
{
	if (customDelayEvents.empty()) return;

	float currentTime = GetTotalTime();

	//for (int i = customDelayEvents.size() - 1; i >= 0; --i)
	//{
	//	if (currentTime >= customDelayEvents[i].eventEndTime)
	//	{
	//		std::string typeName = customDelayEvents[i].eventType;
	//		auto list = customEventList.find(typeName);
	//		if (list != customEventList.end())
	//		{
	//			//for (auto& K : customEventList[typeName])
	//			//{
	//			//	auto targetModule = K->GetModuleToEngine(GetType("LuaScript"));
	//			//	auto lua = static_cast<LuaScript*>(targetModule);
	//			//	lua->CustomEventCall(typeName, customDelayEvents[i].eventTable);
	//			//}
	//		}
	//
	//		if (i != customDelayEvents.size() - 1)
	//		{
	//			customDelayEvents[i] = customDelayEvents.back();
	//			customDelayEvents.pop_back();
	//		}
	//	}
	//}
}

void EventManager::Release()
{
	//for (auto K : eventList)
	//{
	//	K.second.clear();
	//}
	//eventList.clear();
}

void EventManager::Clear()
{
	//for (auto K : eventList)
	//{
	//	K.second.clear();
	//}
	//for (auto K : customEventList)
	//{
	//	K.second.clear();
	//}
	//customDelayEvents.clear();
	//eventList.clear();
	//customEventList.clear();
}

//void EventManager::RegisterMessageCustom(GameObject* target, std::string EventName)
//{
//	auto& list = customEventList[EventName];
//	auto find = std::find(list.begin(), list.end(), target);
//	if (find == list.end())
//	{
//		list.push_back(target);
//	}
//	else
//	{
//		//PixelLog::Error("[Event] 이미 등록된 오브젝트 :" + target->name);
//	}
//}

//void EventManager::UnregisterMessageCustom(GameObject* target, std::string EventName)
//{
//	auto list = customEventList.find(EventName);
//	if (list != customEventList.end())
//	{
//		auto find = std::find(customEventList[EventName].begin(), customEventList[EventName].end(), target);
//		if (find != customEventList[EventName].end())
//		{
//			customEventList[EventName].erase(find);
//		}
//	}
//}

//void EventManager::RegisterMessage(GameObject* target, EventType type)
//{
//	auto& list = eventList[type];
//	auto find = std::find(list.begin(), list.end(), target);
//	if (find == list.end())
//	{
//		list.push_back(target);
//	}
//	else
//	{
//		//PixelLog::Error("[Event] 이미 등록된 오브젝트 :" + target->name);
//	}
//}

//void EventManager::UnregisterMessage(GameObject* target, EventType type)
//{
//	auto list = eventList.find(type);
//	if (list != eventList.end())
//	{
//		auto find = std::find(eventList[type].begin(), eventList[type].end(), target);
//		if (find != eventList[type].end())
//		{
//			eventList[type].erase(find);
//		}
//	}
//}

void EventManager::TriggerCustomEvent(std::string eventType, sol::table event, float time)
{
	//if (time == 0.0f)
	//{
	//	auto list = customEventList.find(eventType);
	//	if (list != customEventList.end())
	//	{
	//		//for (auto& K : customEventList[eventType])
	//		//{
	//		//	auto targetModule = K->GetModuleToEngine(GetType("LuaScript"));
	//		//	auto lua = static_cast<LuaScript*>(targetModule);
	//		//	lua->CustomEventCall(eventType, event);
	//		//}
	//	}
	//}
	//else
	//{
	//	CustomDelayEvent delayEvent;
	//	delayEvent.eventType = eventType;
	//	delayEvent.eventTable = event;
	//	delayEvent.eventStartTime = GetTotalTime();
	//	delayEvent.eventEndTime = GetTotalTime() + time;
	//	customDelayEvents.push_back(delayEvent);
	//}
}

void EventManager::TriggerEvent(EventType type, Event event)
{
	///auto list = eventList.find(type);
	///if (list != eventList.end())
	///{
	///	//for (auto& K : eventList[type])
	///	//{
	///	//	auto targetModule = K->GetModuleToEngine(GetType("LuaScript"));
	///	//	auto lua = static_cast<LuaScript*>(targetModule);
	///	//	lua->EventCall(type, event);
	///	//}
	///}
}
