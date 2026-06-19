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

	return scene->GetNowScene()->CreateGroupEntity(poolNameSTR, scriptNameSTR);
}
