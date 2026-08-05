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
	if (lua == nullptr) lua = Engine->GetFactory<LuaManager>();

	auto luaInfo = lua->GetModuleLua(scriptName);
	if (luaInfo == nullptr)
	{
		PixelLog::Error("Not Find Lua File :" + scriptName);
		return;
	}
	instance = luaInfo->Create();
	this->ID = ID;
	instance["ID"] = ID;
	instance["Active"] = Active;
	lua->AddEntityID(ID, instance);

	OnCollisionEnterFunc = instance["OnCollisionEnter"];
	OnCollisionExitFunc = instance["OnCollisionExit"];
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
	instance["Active"] = Active;

	if (Rigidbody2D_Has(ID))
	{
		Rigidbody2D_SetActive(ID, isActive);
	}
	ActiveEntity(ID, isActive);
}

void ECS::EntityObject::SetChild(unsigned int TargetID)
{
	//이미 자식으로 들어있는지 확인
	auto found = std::find(children.begin(), children.end(), TargetID);
	if (found != children.end())
	{
		PixelLog::Error("[SetChild]The child already exists.");
		return;
	}

	//자식으로 넣기
	children.push_back(TargetID);

	//해당 자식의 부모 설정
	auto child = FindEntity(TargetID);
	child->ParentID = ID;
}

void ECS::EntityObject::SetParent(unsigned int TargetID)
{
	this->ParentID = TargetID;
	auto parent = FindEntity(TargetID);
	if (parent != nullptr)
	{
		parent->SetChild(ID);
	}
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
	sol::object value = instance[memberName];
	return (value.valid()) ? value : sol::nil;
}


void ECS::EntityObject::SetValue(const char* memberName, sol::object value)
{
	if (memberName == nullptr)
	{
		return;
	}
	instance[memberName] = value;
}




