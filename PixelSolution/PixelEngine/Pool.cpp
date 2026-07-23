#include "pch.h"
#include "Pool.h"
#include "PixelEngine.h"
#include "SceneObject.h"
#include "SceneManager.h"
#include "PoolManager.h"

extern PixelEngine* Engine;
unsigned int Pool_Active(const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	return manager->Active(scriptNameSTR);
}

void Pool_SetExpansionSize(const char* scriptName, int CreateCount)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->SetExpansionSize(scriptNameSTR, CreateCount);
}

void Pool_Disable(const char* scriptName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->Disable(scriptNameSTR, id);
}

void Pool_Clear(const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->Clear(scriptNameSTR);
}

int Pool_GetActiveCount(const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	return manager->ActiveCount(scriptNameSTR);
}

unsigned int Pool_GetActiveID(const char* scriptName, int index)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	return manager->ActiveID(scriptNameSTR, index);
}
