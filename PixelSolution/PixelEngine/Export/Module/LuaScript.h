#pragma once
#include "Core/Module.h"
#include "Event.h"
#include <sol/forward.hpp>
enum class EventType : int;
class GameObject;
class LuaModuleInfo;
class LuaManager;
class LuaScript : public Module
{
public:
	LuaScript();
	virtual ~LuaScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	void MessageHub();
	void Reload();
	void Register(std::string fileName);
	void EventCall(EventType type, Event event);
private:
	std::string luaFileName;
	sol::table instance;
	sol::function awake;
	sol::function start;
	sol::function update;
	sol::function message;

	sol::function keyDown;
	sol::function keyUp;

	static LuaManager* lua;
};

