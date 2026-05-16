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

}

unsigned int ECS::Entity::GetID()
{
	return ID;
}



