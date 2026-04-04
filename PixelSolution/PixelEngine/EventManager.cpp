#include "pch.h"
#include "EventManager.h"
#include <algorithm>
#include "Core/GameObject.h"
#include "Module/LuaScript.h"
#include "PixelMetaAPI.h"

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

}

void EventManager::Release()
{
	for (auto K : eventList)
	{
		K.second.clear();
	}
	eventList.clear();
}

void EventManager::Clear()
{
	for (auto K : eventList)
	{
		K.second.clear();
	}
	eventList.clear();
}

void EventManager::RegisterMessage(GameObject* target, EventType type)
{
	auto& list = eventList[type];
	auto find = std::find(list.begin(), list.end(), target);
	if (find == list.end())
	{
		list.push_back(target);
	}
	else 
	{
		PixelLog::Error("[Event] 이미 등록된 오브젝트 :" + target->name);
	}
}

void EventManager::UnregisterMessage(GameObject* target, EventType type)
{
	auto list = eventList.find(type);
	if (list != eventList.end())
	{
		auto find = std::find(eventList[type].begin(), eventList[type].end(), target);
		if (find != eventList[type].end())
		{
			eventList[type].erase(find);
		}
	}
}

void EventManager::TriggerEvent(EventType type, Event event)
{
	auto list = eventList.find(type);
	if (list != eventList.end())
	{
		for (auto& K : eventList[type])
		{
			auto targetModule =  K->GetModuleToEngine(GetType("LuaScript"));
			auto lua = static_cast<LuaScript*>(targetModule);
			lua->EventCall(type, event);
		}
	}
}
