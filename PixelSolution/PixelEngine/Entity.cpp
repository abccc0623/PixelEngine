#include "pch.h"
#include "Entity.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "LuaManager.h"
#include "SceneObject.h"

extern PixelEngine* Engine;
unsigned int Entity_Create(const char* scriptName)
{
	// Lua에서 nil을 넘겼을 때 std::string(nullptr)가 네이티브 예외를 일으키지 않게 막는다.
	if (scriptName == nullptr)
	{
		PixelLog::Error("[Entity][Create] scriptName is nil.");
		return UINT32_MAX;
	}
	std::string scriptNameSTR(scriptName);
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto luaManager = Engine->GetFactory<LuaManager>();
	auto id = sceneManager->CreateEntity(scriptNameSTR);
	return id;
}

unsigned int Entity_CreateEmpty()
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	return sceneManager->CreateEntity("");
}

void Entity_Destroy(unsigned int id)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	uint32_t u(id);
	sceneManager->DestroyEntity(u);
}

void Entity_SetActive(unsigned int id, bool active)
{
	auto find = FindEntity(id);
	if (find == nullptr)
	{
		PixelLog::Error("[Entity][SetActive] Invalid Entity ID: " + std::to_string(id));
		return;
	}
	find->SetActive(active);
}

bool Entity_GetActive(unsigned int id)
{
	auto find = FindEntity(id);
	if (find == nullptr)
	{
		PixelLog::Error("[Entity][GetActive] Invalid Entity ID: " + std::to_string(id));
		return false;
	}
	return find->GetActive();
}

sol::object Entity_GetValue(unsigned int id, const char* memberName)
{
	auto target = FindEntity(id);
	if (target == nullptr)
	{
		return sol::nil;
	}
	return target->GetValue(memberName);
}

void Entity_SetValue(unsigned int id, const char* memberName, sol::object value)
{
	auto target = FindEntity(id);
	if (target == nullptr)
	{
		PixelLog::Error("[Entity][SetValue] Not Find ID");
		return;
	}
	target->SetValue(memberName, value);
}
