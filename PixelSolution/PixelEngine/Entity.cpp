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
	find->SetActive(active);
}

bool Entity_GetActive(unsigned int id)
{
	auto find = FindEntity(id);
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
