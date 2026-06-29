#include "pch.h"
#include "Pool.h"
#include "PixelEngine.h"
#include "Scene.h"
#include "SceneManager.h"
#include "PoolManager.h"

extern PixelEngine* Engine;
unsigned int ECS::Pool::Active(const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	return manager->Active(scriptNameSTR);
}

void ECS::Pool::Disable(const char* scriptName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->Disable(scriptNameSTR, id);
}

void ECS::Pool::Clear(const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->Clear(scriptNameSTR);
}

sol::as_table_t<std::vector<unsigned int>> ECS::Pool::GetActiveArray(const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string scriptNameSTR(scriptName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	return manager->GetActiveArray(scriptNameSTR);
}
