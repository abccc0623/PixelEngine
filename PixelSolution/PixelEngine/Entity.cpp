#include "pch.h"
#include "Entity.h"
#include <sol/sol.hpp>
#include "PixelEngine.h"
#include "LuaManager.h"
#include "LuaModuleInfo.h"

extern PixelEngine* Engine;
LuaManager* ECS::Entity::lua = nullptr;
ECS::Entity::Entity()
{
	scriptName = "";
	ID = -1;
}
ECS::Entity::~Entity()
{

}
void ECS::Entity::Create(const std::string& scriptName, unsigned int ID)
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
	//엔진에서 루아를 생성할 떄 루아매니저도 생성
	//루아매니저에서 생성된 모든 Entity를 등록 및 삭제(기본 함수도 호출)
	instance = luaInfo->Create();
	instance["ID"] = ID;
	lua->AddEntityID(ID, instance);

	OnCollisionEnterFunc = instance["OnCollisionEnter"];
	OnCollisionExitFunc = instance["OnCollisionExit"];
}

void ECS::Entity::OnCollisionEnter(unsigned int TargetID)
{
	if (OnCollisionEnterFunc.valid())
	{
		auto result = OnCollisionEnterFunc(instance, TargetID);
		if (!result.valid())
		{
			sol::error err = result;
			std::string errorMsg = err.what();
			PixelLog::Error(errorMsg.c_str());
		}
	}
}

void ECS::Entity::OnCollisionExit(unsigned int TargetID)
{
	if (OnCollisionExitFunc.valid())
	{
		auto result = OnCollisionExitFunc(instance, TargetID);
		if (!result.valid())
		{
			sol::error err = result;
			std::string errorMsg = err.what();
			PixelLog::Error(errorMsg.c_str());
		}

	}
}

void ECS::Entity::OnEvent(std::string eventName, sol::table event)
{
	auto luaEvent = instance[eventName];
	if (luaEvent.valid())
	{
		auto result = luaEvent(instance);
		if (!result.valid())
		{
			sol::error err = result;
			std::string errorMsg = err.what();
			PixelLog::Error(errorMsg.c_str());
		}
	}
}

unsigned int ECS::Entity::GetID()
{
	return ID;
}



