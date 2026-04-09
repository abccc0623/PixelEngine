#include "pch.h"
#include "LuaScript.h"
#include "sol.hpp"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "LuaModuleInfo.h"
#include "LuaManager.h"
#include "EventManager.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Type/GlobalEnum.h"
#include "Log.h"
extern PixelEngine* Engine;
LuaManager* LuaScript::lua = nullptr;
EventManager* LuaScript::event = nullptr;
LuaScript::LuaScript()
{
    if (lua == nullptr)
    {
        lua = Engine->GetFactory<LuaManager>();
        event = Engine->GetFactory<EventManager>();
    }
}

LuaScript::~LuaScript()
{
    //instance = nullptr;
}

void LuaScript::Awake()
{
    if (awake.valid())
    {
    	auto result = awake(instance);
    	if (!result.valid())
    	{
    		sol::error err = result;
    		std::string what = err.what();
    		PixelLog::Error("--- LUA AWAKE ERROR ---");
    		PixelLog::Error(what);
    		PixelLog::Error("-----------------------");
    	}
    }

}

void LuaScript::Start()
{
    if (start.valid())
    {
    	auto result = start(instance);
    	if (!result.valid())
    	{
    		sol::error err = result;
    		std::string what = err.what();
    		PixelLog::Error("--- LUA START ERROR ---");
    		PixelLog::Error(what);
    		PixelLog::Error("-----------------------");
    	}
    }
}

void LuaScript::Update()
{
    if (update.valid())
    {
        auto result = update(instance, GetDeltaTime());
        if (!result.valid())
        {
            sol::error err = result;
            std::string what = err.what();
            PixelLog::Error("--- LUA UPDATE ERROR ---");
            PixelLog::Error(what);
            PixelLog::Error("-----------------------");
        }
    }
}

void LuaScript::Reload()
{
    if (update.valid())
    {
        auto result = update(instance, GetDeltaTime());
        if (!result.valid())
        {
            sol::error err = result;
            std::string what = err.what();
            PixelLog::Error("--- LUA SRART ERROR ---");
            PixelLog::Error(what);
            PixelLog::Error("-----------------------");
        }
    }
}

void LuaScript::Register(std::string fileName)
{
    auto luaInfo = lua->GetModuleLua(fileName);
    if (luaInfo == nullptr) 
    {
        PixelLog::Error("Not Find Lua File :" + fileName);
        return;
    }
    luaFileName = fileName;
    instance = luaInfo->Create();
    
    awake = instance["Awake"];
    start = instance["Start"];
    update = instance["Update"];

    keyDown     = instance["KeyDownEvent"];
    keyUp       = instance["KeyUpEvent"];
    CustomEvent = instance["CustomEvent"];
    
    instance["gameObject"] = targetObject;
    instance["transform"] = transform;
    instance["script"] = this;
}

void LuaScript::EventCall(EventType type, Event event)
{
    switch (type)
    {
    case EventType::KEY_DOWN:
        if (keyDown.valid()){keyDown(instance, event.key);}
        break;
    case EventType::KEY_UP:
        if (keyUp.valid()) { keyUp(instance, event.key); }
        break;
    }
}
void LuaScript::TriggerCustomEvent(std::string eventType, sol::table eventTabel, float time)
{
    event->TriggerCustomEvent(eventType, eventTabel, time);
}
void LuaScript::CustomEventCall(std::string eventType,sol::table eventTabel)
{
    if (CustomEvent.valid())
    {
        CustomEvent(instance, eventType, eventTabel);
    }
    else
    {
        PixelLog::Warn("해당 오브젝트의 Event함수가 없습니다.:" + eventType);
    }
}

void LuaScript::RegisterMessage(EventType type)
{
    event->RegisterMessage(targetObject, type);
}

void LuaScript::RegisterCustomMessage(const char* type)
{
    event->RegisterMessageCustom(targetObject, type);
}

void LuaScript::UnregisterMessage(EventType type)
{
    event->UnregisterMessage(targetObject, type);
}

void LuaScript::UnregisterCustomMessage(const char* type)
{
    event->UnregisterMessageCustom(targetObject, type);
}


sol::table LuaScript::Get()
{
    if (!instance.valid())
    {
        PixelLog::Error("LuaScript -> Get 유효 하지 않습니다.");
        return sol::lua_nil;
    }
    return instance;
}
