#include "pch.h"
#include "SystemManager.h"
#include "PixelEngine.h"
#include "TimeManager.h"

#include "TransformSystem.h"
#include "CameraSystem.h"
#include "Rigidbody2DSystem.h"
#include "Animation2DSystem.h"
#include "UIImageSystem.h"
#include "UITextSystem.h"
#include "GraphicsSystem.h"

#include "Registry.h"
#include "PixelEngineAPI.h"

ECS::SystemManager::SystemManager() {}
ECS::SystemManager::~SystemManager() {}

void ECS::SystemManager::Initialize()
{
	SystemList.push_back(new TransformSystem());
	SystemList.push_back(new CameraSystem());
	SystemList.push_back(new Animation2DSystem);
	SystemList.push_back(new Rigidbody2DSystem);

	SystemList.push_back(new UIImageSystem);
	//SystemList.push_back(new UITextSystem);
	SystemList.push_back(new GraphicsSystem);
}

void ECS::SystemManager::Update(ECS::Registry* registry)
{
	int size = SystemList.size();
	if (IsPlayMode())
	{
		for (int i = 0; i < size; i++)
		{
			SystemList[i]->Update(registry);
		}
	}
	else
	{
		for (int i = 0; i < size; i++)
		{
			SystemList[i]->EditorUpdate(registry);
		}
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
