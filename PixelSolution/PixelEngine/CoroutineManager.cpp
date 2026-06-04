#include "pch.h"
#include "CoroutineManager.h"
#include "PixelEngineAPI.h"

CoroutineManager::CoroutineManager()
{

}

CoroutineManager::~CoroutineManager()
{

}

void CoroutineManager::Initialize()
{

}

void CoroutineManager::Update()
{
	float dt = GetDeltaTime();

	for (auto it = CoroutineList.begin(); it != CoroutineList.end(); )
	{
		auto& info = *it;

		if (info->isWaiting)
		{
			info->waitTimer -= dt;
			if (info->waitTimer <= 0)
			{
				info->isWaiting = false;
				if (info->co.status() == sol::call_status::yielded)
				{
					// Resume without arguments after a timed wait.
					auto result = info->co();
					if (!result.valid())
					{
						sol::error err = result;
						PixelLog::Error("Coroutine Runtime Error: " + std::string(err.what()));
						it = CoroutineList.erase(it);
						continue;
					}
				}
			}
		}
		if (info->co.status() != sol::call_status::yielded)
		{
			it = CoroutineList.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void CoroutineManager::Release()
{
	CoroutineList.clear();
}

void CoroutineManager::Clear()
{
	CoroutineList.clear();
}

void CoroutineManager::MarkAsWaiting(sol::this_state s, float seconds)
{
	lua_State* currentL = s.lua_state();
	for (auto& info : CoroutineList)
	{
		if (info->co.lua_state() == currentL)
		{
			info->waitTimer = seconds;
			info->isWaiting = true;
			break;
		}
	}
}

void CoroutineManager::Add(const char* functionName, sol::thread thread, sol::coroutine co)
{
	auto CoroutineObj = std::make_shared<LuaCoroutine>();
	CoroutineObj->name = functionName;
	CoroutineObj->luaThread = std::move(thread);
	CoroutineObj->co = co;
	CoroutineObj->isWaiting = false;

	CoroutineList.push_back(CoroutineObj);
}

void CoroutineManager::Start(const std::string& functionName, sol::protected_function function, sol::table self, sol::object argument)
{
	if (!function.valid())
	{
		return;
	}

	sol::thread thread = sol::thread::create(function.lua_state());
	sol::coroutine co(thread.thread_state(), function);

	auto coroutineObj = std::make_shared<LuaCoroutine>();
	coroutineObj->name = functionName;
	coroutineObj->luaThread = std::move(thread);
	coroutineObj->co = co;
	coroutineObj->isWaiting = false;
	CoroutineList.push_back(coroutineObj);

	auto result = (argument.valid() && argument != sol::nil) ? coroutineObj->co(self, argument) : coroutineObj->co(self);
	if (!result.valid())
	{
		sol::error err = result;
		PixelLog::Error("Coroutine Runtime Error: " + std::string(err.what()));
		CoroutineList.remove(coroutineObj);
		return;
	}

	if (coroutineObj->co.status() != sol::call_status::yielded)
	{
		CoroutineList.remove(coroutineObj);
	}
}
