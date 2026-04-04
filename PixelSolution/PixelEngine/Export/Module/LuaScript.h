#pragma once
#include "Core/Module.h"
#include "Event.h"
#include <sol/forward.hpp>
enum class EventType : int;
class GameObject;
class LuaModuleInfo;
class LuaManager;
class EventManager;
class LuaScript : public Module
{
public:
	LuaScript();
	virtual ~LuaScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	void Reload();
	void Register(std::string fileName);
	void EventCall(EventType type, Event event);
	void TriggerCustomEvent(std::string eventType, sol::table event,float time);
	void CustomEventCall(std::string eventType, sol::table event);
	

	void RegisterMessage(EventType type);
	void RegisterCustomMessage(const char* type);
	void UnregisterMessage(EventType type);
	void UnregisterCustomMessage(const char* type);

	sol::table Get();
private:
	std::string luaFileName;
	sol::table instance;
	sol::function awake;
	sol::function start;
	sol::function update;
	sol::function message;

	sol::function keyDown;
	sol::function keyUp;
	sol::function CustomEvent;

	static LuaManager* lua;
	static EventManager* event;
};

