#pragma once
#include "EngineManager.h"
#include <unordered_map>
#include <vector>

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

	void RegisterMessage(GameObject* target, EventType type);
	void UnregisterMessage(GameObject* target, EventType type);


	template<typename... Args>
	void TriggerEvent(EventType type, Args... args)
	{
		auto it = eventList.find(type);
		if (it == eventList.end()) return;

		for (auto& callback : it->second)
		{
			
		}
	}
	
private:
	std::unordered_map<EventType, std::vector<GameObject*>> eventList;
};

