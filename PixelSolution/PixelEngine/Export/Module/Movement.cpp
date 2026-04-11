#include "pch.h"
#include "Movement.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "Module/Transform.h"
#include "Module/LuaScript.h"
#include "Core/GameObject.h"
#include "Type/PVector3.h"
#include "Scene.h"
extern PixelEngine* Engine;
Movement::Movement()
{
	bitmask.set(NONE);
}

Movement::~Movement()
{

}

void Movement::Update()
{
	if (target == nullptr)return;

	if (target.IsValid())
	{
		auto k  = target.Lock();
		auto tr = k->GetTransform();
		
		auto distance =  Distance(transform->Position, tr->Position);
		auto direction = tr->Position - transform->Position;
		direction = direction.Normalize();

		if (distance > stopDistance)
		{
			//계속 이동해야 할떄
			transform->Position += direction * (GetDeltaTime() * speed);
			bitmask.set(MOVE);
		}
		else
		{
			//멈춰야 할 때
			bitmask.reset(MOVE);
		}

		//이동중이고 START 비트가 켜져 있지 않으면
		if (bitmask.hasAny(MOVE) == true && bitmask.hasAny(START) == false)
		{
			CallFunction(started);
			bitmask.set(START);
			bitmask.reset(END);
		}

		if (bitmask.hasAny(MOVE) == false && bitmask.hasAny(END) == false)
		{
			//현재 이동X 이전에 complete 함수가 호출안됬다면
			if (bitmask.hasAny(START))
			{
				CallFunction(complete);
				bitmask.set(END);
				bitmask.reset(START);
			}
		}
	}
}


void Movement::MoveToTarget(GameObject* target)
{
	bitmask.reset(STOP);
	auto manager = Engine->GetFactory<SceneManager>();
	auto scene =  manager->GetNowScene();
	this->target = scene->FindGameObjectToEngine(target);
}

void Movement::StopMove()
{
	bitmask.set(STOP);
}

void Movement::AddCompleteCallBack(std::string functionName)
{
	Module* targetModule = this->GetGameObject()->GetModuleToEngine("LuaScript");
	if (targetModule != nullptr)
	{
		LuaScript* script = static_cast<LuaScript*>(targetModule);
		instance = script->Get();
		complete = instance[functionName];
	}
}

void Movement::AddStartedCallBack(std::string functionName)
{
	Module* targetModule = this->GetGameObject()->GetModuleToEngine("LuaScript");
	if (targetModule != nullptr)
	{
		LuaScript* script = static_cast<LuaScript*>(targetModule);
		instance = script->Get();
		started = instance[functionName];
	}
}

void Movement::CallFunction(sol::function& call)
{
	if (call.valid())
	{
		auto result = call(instance);
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			PixelLog::Error(what);
		}
	}
}
