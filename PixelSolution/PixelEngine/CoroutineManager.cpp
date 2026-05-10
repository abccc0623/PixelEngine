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
	float dt = GetDeltaTime(); // 엔진에서 제공하는 DeltaTime 함수

	for (auto it = CoroutineList.begin(); it != CoroutineList.end(); )
	{
		auto& info = *it;

		if (info->isWaiting)
		{
			info->waitTimer -= dt;
			if (info->waitTimer <= 0)
			{
				info->isWaiting = false;

				// 루아 스택이 살아있고 중단된 상태인지 확인 후 재개
				if (info->co.status() == sol::call_status::yielded)
				{
					// 재개 시에는 인자를 넣지 않습니다.
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

		// 코루틴이 끝났으면 리스트에서 삭제
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

}

void CoroutineManager::Clear()
{
	CoroutineList.clear();
}

void CoroutineManager::MarkAsWaiting(sol::this_state s, float seconds)
{
	lua_State* currentL = s.lua_state();

	// 관리 리스트(std::list<std::shared_ptr<LuaCoroutine>>)에서 해당 코루틴 찾기
	for (auto& info : CoroutineList)
	{
		// info는 shared_ptr이므로 -> 연산자로 접근합니다.
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
	CoroutineObj->luaThread = std::move(thread); // 스레드 소유권 이전
	CoroutineObj->co = co;
	CoroutineObj->isWaiting = false;

	CoroutineList.push_back(CoroutineObj);
}
