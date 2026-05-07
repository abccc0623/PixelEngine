#include "pch.h"
#include "SystemManager.h"
#include "ModuleSystem.h"
#include "PixelEngine.h"
#include "TimeManager.h"

//System
#include "TransformSystem.h"

extern PixelEngine* Engine;
SystemManager::SystemManager()
{
	SystemList = std::vector<ModuleSystem*>();
	timeManager = Engine->GetFactory<TimeManager>();

	SystemList.push_back(new TransformSystem());
}

SystemManager::~SystemManager()
{
	for (int i = 0; i < SystemList.size(); i++)
	{
		delete SystemList[i];
		SystemList[i] = nullptr;
	}
}

void SystemManager::Initialize()
{
	for (int i = 0; i < SystemList.size(); i++)
	{
		SystemList[i]->Initialize();
	}
}

void SystemManager::Update()
{
	float time =  timeManager->GetDeltaTime();
	for (int i = 0; i < SystemList.size(); i++)
	{
		SystemList[i]->Update(time);
	}
}

void SystemManager::Release()
{
	for (int i = 0; i < SystemList.size(); i++)
	{
		SystemList[i]->Clear();
	}
}
void SystemManager::Clear()
{
	for (int i = 0; i < SystemList.size(); i++)
	{
		SystemList[i]->Clear();
	}
}
