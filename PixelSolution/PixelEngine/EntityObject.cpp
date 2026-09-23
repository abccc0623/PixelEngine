#include "pch.h"
#include "EntityObject.h"
#include <sol/sol.hpp>
#include "PixelEngine.h"
#include "LuaManager.h"
#include "LuaModuleInfo.h"
#include "CoroutineManager.h"
#include "PhysManager.h"
#include "Rigidbody2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"

extern PixelEngine* Engine;
LuaManager* ECS::EntityObject::lua = nullptr;
ECS::EntityObject::EntityObject()
{
	scriptName = "";
	ID = -1;
}
ECS::EntityObject::~EntityObject()
{

}
void ECS::EntityObject::Create(const std::string& scriptName, unsigned int ID)
{
	this->scriptName = scriptName;
	Active = true;
	this->ID = ID;

	if (scriptName.empty())
	{
		return;
	}

	if (lua == nullptr) lua = Engine->GetFactory<LuaManager>();

	auto luaInfo = lua->GetModuleLua(scriptName);
	if (luaInfo == nullptr)
	{
		PixelLog::Error("Not Find Lua File :" + scriptName);
		return;
	}
	instance = luaInfo->Create();
	instance["ID"] = ID;
	instance["Active"] = Active;
	lua->AddEntityID(ID, instance);

	// 존재하지 않는 Lua 필드를 protected_function으로 강제 변환해 모든 Entity가 nil 레퍼런스를
	// 보관하지 않도록 한다. 실제 함수가 선언된 스크립트만 콜백을 Registry에 유지한다.
	sol::object collisionEnter = instance["OnCollisionEnter"];
	if (collisionEnter.is<sol::protected_function>())
	{
		OnCollisionEnterFunc = collisionEnter.as<sol::protected_function>();
	}

	sol::object collisionExit = instance["OnCollisionExit"];
	if (collisionExit.is<sol::protected_function>())
	{
		OnCollisionExitFunc = collisionExit.as<sol::protected_function>();
	}

	sol::object animationCallback = instance["OnAnimationCallBack"];
	if (animationCallback.is<sol::protected_function>())
	{
		OnAnimationCallBackFunc = animationCallback.as<sol::protected_function>();
	}
}

void ECS::EntityObject::OnCollisionEnter(unsigned int TargetID)
{
	sol::protected_function collisionFunc = OnCollisionEnterFunc;
	if (collisionFunc.valid())
	{
		auto coroutine = Engine->GetFactory<CoroutineManager>();
		sol::object targetID = sol::make_object(instance.lua_state(), TargetID);
		coroutine->Start("OnCollisionEnter", collisionFunc, instance, targetID);
	}
}

void ECS::EntityObject::OnCollisionExit(unsigned int TargetID)
{
	sol::protected_function collisionFunc = OnCollisionExitFunc;
	if (collisionFunc.valid())
	{
		auto coroutine = Engine->GetFactory<CoroutineManager>();
		sol::object targetID = sol::make_object(instance.lua_state(), TargetID);
		coroutine->Start("OnCollisionExit", collisionFunc, instance, targetID);
	}
}

void ECS::EntityObject::OnAnimationCallBack(unsigned int TargetID, int nowAnimationID, int nowAnimationFrame)
{
	if (!OnAnimationCallBackFunc.valid()) return;

	auto result = OnAnimationCallBackFunc(instance, TargetID, nowAnimationID, nowAnimationFrame);
	if (!result.valid())
	{
		sol::error err = result;
		PixelLog::Error("Animation callback runtime error: " + std::string(err.what()));
	}
}

void ECS::EntityObject::OnEvent(const std::string& functionName)
{
	if (!instance.valid()) return;

	sol::object functionObject = instance[functionName];
	if (!functionObject.is<sol::protected_function>()) return;

	auto coroutine = Engine->GetFactory<CoroutineManager>();
	if (coroutine == nullptr) return;

	auto function = functionObject.as<sol::protected_function>();
	auto emptyArgument = sol::make_object(instance.lua_state(), sol::nil);
	coroutine->Start(functionName, function, instance, emptyArgument);
}

void ECS::EntityObject::OnEvent(std::string functionName, sol::object event)
{
	sol::protected_function luaEvent = instance[functionName];
	if (luaEvent.valid())
	{
		auto coroutine = Engine->GetFactory<CoroutineManager>();
		coroutine->Start(functionName, luaEvent, instance, event);
	}
}

unsigned int ECS::EntityObject::GetID()
{
	return ID;
}

bool ECS::EntityObject::GetActive()
{
	return Active;
}

void ECS::EntityObject::SetActive(bool isActive)
{
	Active = isActive;
	if (instance.valid())
	{
		instance["Active"] = Active;
	}

	if (Rigidbody2D_Has(ID))
	{
		Rigidbody2D_SetActive(ID, isActive);
	}
	ActiveEntity(ID, isActive);

	for (auto childID : children)
	{
		auto child = FindEntity(childID);
		if (child != nullptr)
		{
			child->SetActive(isActive);
		}
	}
}

void ECS::EntityObject::SetChild(unsigned int TargetID)
{
	// 자기 자신을 자식으로 등록하면 Transform/Active 재귀 호출이 무한 반복된다.
	if (TargetID == ID)
	{
		PixelLog::Error("[EntityObject][SetChild] Cannot register itself as a child. ID: " + std::to_string(ID));
		return;
	}

	//이미 자식으로 들어있는지 확인
	auto found = std::find(children.begin(), children.end(), TargetID);
	if (found != children.end())
	{
		PixelLog::Error("[SetChild]The child already exists.");
		return;
	}

	// 존재하지 않는 ID를 바로 역참조하면 네이티브 크래시가 발생한다.
	auto child = FindEntity(TargetID);
	if (child == nullptr)
	{
		PixelLog::Error("[EntityObject][SetChild] Invalid child ID: " + std::to_string(TargetID));
		return;
	}

	// 부모의 상위 계층을 자식으로 연결하면 순환 계층이 만들어진다.
	for (auto ancestor = this; ancestor != nullptr;)
	{
		if (ancestor->GetID() == TargetID)
		{
			PixelLog::Error("[EntityObject][SetChild] Circular hierarchy is not allowed. Parent ID: " +
				std::to_string(ID) + ", Child ID: " + std::to_string(TargetID));
			return;
		}

		const unsigned int ancestorParentID = ancestor->GetParentID();
		ancestor = ancestorParentID == UINT32_MAX ? nullptr : FindEntity(ancestorParentID);
	}

	// 기존 부모가 있다면 그 부모의 자식 목록에서 먼저 제거한다.
	if (child->ParentID != UINT32_MAX)
	{
		auto oldParent = FindEntity(child->ParentID);
		if (oldParent != nullptr)
		{
			auto& oldChildren = oldParent->children;
			oldChildren.erase(std::remove(oldChildren.begin(), oldChildren.end(), TargetID), oldChildren.end());
		}
	}

	//자식으로 넣기
	children.push_back(TargetID);

	//해당 자식의 부모 설정
	child->ParentID = ID;
}

void ECS::EntityObject::SetParent(unsigned int TargetID)
{
	// 자기 자신을 부모로 등록하면 계층 순환이 생긴다.
	if (TargetID == ID)
	{
		PixelLog::Error("[EntityObject][SetParent] Cannot register itself as a parent. ID: " + std::to_string(ID));
		return;
	}

	auto parent = FindEntity(TargetID);
	if (parent == nullptr)
	{
		PixelLog::Error("[EntityObject][SetParent] Invalid parent ID: " + std::to_string(TargetID));
		return;
	}

	parent->SetChild(ID);
}

unsigned int ECS::EntityObject::GetParentID()
{
	return ParentID;
}

std::vector<unsigned int>& ECS::EntityObject::GetChild()
{
	return children;
}




sol::object ECS::EntityObject::GetValue(const char* memberName)
{
	if (memberName == nullptr)
	{
		return sol::nil;
	}
	if (!instance.valid())
	{
		return sol::nil;
	}
	sol::object value = instance[memberName];
	return (value.valid()) ? value : sol::nil;
}


void ECS::EntityObject::SetValue(const char* memberName, sol::object value)
{
	if (memberName == nullptr)
	{
		return;
	}
	if (!instance.valid())
	{
		return;
	}
	instance[memberName] = value;
}




