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

	if (ECS::Rigidbody2D::HasComponent(ID))
	{
		auto registry = GetRegistry();
		PhysManager* phys = Engine->GetFactory<PhysManager>();
		ECS::Rigidbody2D::Rigidbody2DData* data = registry->Get<ECS::Rigidbody2D::Rigidbody2DData>(ID);
		phys->SetActive(JPH::BodyID(data->bodyID), isActive);
		data->velocity = { 0,0,0 };
		data->velocityCopy = { 0,0,0 };
	}
	ActiveEntity(ID, isActive);
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




