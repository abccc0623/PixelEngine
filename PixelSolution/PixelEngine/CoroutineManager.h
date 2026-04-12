#pragma once
#include "EngineManager.h"
#include <sol/forward.hpp>
struct LuaCoroutine 
{
    sol::thread luaThread;   // 독립적인 루아 스택 (Garbage Collection 방지용)
    sol::coroutine co;       // 실제 실행 가능한 코루틴 객체
    float waitTimer = 0.0f;  // 남은 대기 시간
    bool isWaiting = false;  // 현재 타이머 대기 중인지 여부
    int threadRef = LUA_NOREF;
    std::string name;        // 디버깅용 이름

    bool IsFinished() const
    {
        // 코루틴 상태가 더 이상 실행 가능하지 않은지 체크
        return co.status() != sol::call_status::yielded;
    }
    ~LuaCoroutine() {
        if (threadRef != LUA_NOREF) {
            // 엔진의 메인 lua_State를 가져와서 해제해야 함
            // luaL_unref(L, LUA_REGISTRYINDEX, threadRef);
        }
    }
};


class CoroutineManager :public EngineManager
{
public:
	CoroutineManager();
	virtual ~CoroutineManager();

	// EngineManager을(를) 통해 상속됨
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;
    void MarkAsWaiting(sol::this_state s,float seconds);
    void Add(const char* functionName, sol::thread thread, sol::coroutine co);
private:
    std::list<std::shared_ptr<LuaCoroutine>> CoroutineList;

};

