#include "pch.h"
#include "SystemManager.h"
#include "PixelEngine.h"
#include "TimeManager.h"

#include "TransformSystem.h"
#include "CameraSystem.h"
#include "Collider2DSystem.h"
#include "Rigidbody2DSystem.h"
#include "Animation2DSystem.h"
#include "Renderer2DSystem.h"

ECS::SystemManager::SystemManager() {}
ECS::SystemManager::~SystemManager() {}

void ECS::SystemManager::Initialize()
{
	SystemList.push_back(new TransformSystem());
	SystemList.push_back(new CameraSystem());
	SystemList.push_back(new Animation2DSystem);
	//SystemList.push_back(new Collider2DSystem);
	SystemList.push_back(new Rigidbody2DSystem);
	SystemList.push_back(new Renderer2DSystem);
}

void ECS::SystemManager::Update(ECS::Registry* registry)
{
	int size = SystemList.size();
	for (int i = 0; i < size; i++)
	{
		SystemList[i]->Update(registry);
	}
}

void ECS::SystemManager::Release()
{
	int size = SystemList.size();
	for (int i = 0; i < size; i++)
	{
		SystemList[i]->Release();
		delete SystemList[i];
	}
	SystemList.clear();
}
