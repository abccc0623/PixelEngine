#include "pch.h"
#include "LuaScript.h"
#include "sol.hpp"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "LuaModuleInfo.h"
#include "LuaManager.h"
#include "EventManager.h"
#include "CoroutineManager.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Type/GlobalEnum.h"
#include "Log.h"
extern PixelEngine* Engine;
LuaManager* LuaScript::lua = nullptr;
EventManager* LuaScript::event = nullptr;
CoroutineManager* LuaScript::coroutine = nullptr;
LuaScript::LuaScript()
{
	if (lua == nullptr)
	{
		lua = Engine->GetFactory<LuaManager>();
		event = Engine->GetFactory<EventManager>();
		coroutine = Engine->GetFactory<CoroutineManager>();
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

	keyDown = instance["KeyDownEvent"];
	keyUp = instance["KeyUpEvent"];
	CustomEvent = instance["CustomEvent"];

	OnCollisionIn = instance["OnCollisionIn"];
	OnCollisionOut = instance["OnCollisionOut"];

	instance["gameObject"] = targetObject;
	instance["transform"] = transform;
	instance["script"] = this;
}

void LuaScript::EventCall(EventType type, Event event)
{
	switch (type)
	{
	case EventType::KeyDown:
		if (keyDown.valid()) { keyDown(instance, event.key); }
		break;
	case EventType::KeyUp:
		if (keyUp.valid()) { keyUp(instance, event.key); }
		break;
	case EventType::CollisionIn:
		if (OnCollisionIn.valid())
		{
			if (event.Collision.target1 == targetObject)
			{
				OnCollisionIn(instance, event.Collision.target2);
			}
			else if (event.Collision.target2 == targetObject)
			{
				OnCollisionIn(instance, event.Collision.target1);
			}
			else
			{

			}
		}

		break;
	case EventType::CollisionOut:
		if (OnCollisionOut.valid()) { OnCollisionOut(instance); }
		break;
	}
}
void LuaScript::TriggerCustomEvent(std::string eventType, sol::table eventTabel, float time)
{
	event->TriggerCustomEvent(eventType, eventTabel, time);
}
void LuaScript::CustomEventCall(std::string eventType, sol::table eventTabel)
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

void LuaScript::StartCoroutine(const char* functionName)
{
	sol::protected_function func = instance[functionName];
	if (!func.valid()) return;

	// 새 스레드 생성
	sol::thread runner = sol::thread::create(instance.lua_state());
	sol::state_view runner_state = runner.state();

	// 새 스레드 상태에서 코루틴 객체 추출
	sol::coroutine co = runner_state[functionName] = func;

	// 관리자에 등록
	coroutine->Add(functionName, std::move(runner), co);

	// ⭐ 처음 실행 시 instance(self)를 넘겨서 루아의 Script:함수 문법 대응
	auto result = co(instance);
	if (!result.valid())
	{
		sol::error err = result;
		PixelLog::Error("코루틴 시작 실패: " + std::string(err.what()));
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
