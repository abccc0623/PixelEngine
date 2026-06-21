#include "pch.h"
#include "Pool.h"
#include "PixelEngine.h"
#include "Scene.h"
#include "SceneManager.h"
#include "PoolManager.h"

extern PixelEngine* Engine;
unsigned int ECS::Pool::CreateEntity(const char* poolName, const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string poolNameSTR(poolName);
	std::string scriptNameSTR(scriptName);

	return scene->GetNowScene()->CreatePoolEntity(poolNameSTR, scriptNameSTR);
}

unsigned int ECS::Pool::Active(const char* poolName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string poolNameSTR(poolName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	return manager->Active(poolNameSTR);
}

void ECS::Pool::Disable(const char* poolName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string poolNameSTR(poolName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->Disable(poolNameSTR, id);
}

void ECS::Pool::Clear(const char* poolName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string poolNameSTR(poolName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->Clear(poolNameSTR);
}

void ECS::Pool::SetAutoCreateFunction(const char* poolName, sol::function func)
{
	if (!func.valid())
	{
		PixelLog::Error("[Pool][SetAutoCreateFunction] Invalid function.");
		return;
	}

	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string poolNameSTR(poolName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->SetAutoCreateFunction(poolNameSTR, func);
}

void ECS::Pool::SetAutoActiveFunction(const char* poolName, sol::function func)
{
	if (!func.valid())
	{
		PixelLog::Error("[Pool][SetAutoCreateFunction] Invalid function.");
		return;
	}

	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string poolNameSTR(poolName);

	auto manager = scene->GetNowScene()->GetPoolManager();
	manager->SetAutoActiveFunction(poolNameSTR, func);
}
