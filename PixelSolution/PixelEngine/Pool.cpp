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
