#include "PixelEngine.h"
#include "KeyInputManager.h"
#include "WindowPlatform.h"
#include "TimeManager.h"
#include <Windows.h>

void PixelEngine::Initialize()
{
	ManagerList = new EngineManager*[managerArraySize];
	targetPlatform = new WindowPlatform();
	keyInputManager = new KeyInputManager();
	timeManager = new TimeManager();

	ManagerList[0] = targetPlatform;
	ManagerList[1] = keyInputManager;
	ManagerList[2] = timeManager;
	
	for (int i = 0; i < managerArraySize; i++)
	{
		ManagerList[i]->Initialize();
	}
}

void PixelEngine::Update()
{
	EngineUpdate();
}

void PixelEngine::ClientUpdate()
{
	
}

void PixelEngine::EngineUpdate()
{
	if (ManagerList == nullptr)return;
	for (int i = 0; i < managerArraySize; i++)
	{
		ManagerList[i]->Update();
	}
}

bool PixelEngine::GetKeyDown(byte number)
{
	return keyInputManager->GetKeyDown(number);
}

bool PixelEngine::GetKeyUp(byte number)
{
	return keyInputManager->GetKeyUp(number);
}

bool PixelEngine::GetKey(byte number)
{
	return keyInputManager->GetKey(number);
}

void PixelEngine::Release()
{
	if (ManagerList == nullptr)return;
	for (int i = 0; i < managerArraySize; i++)
	{
		ManagerList[i]->Release();
		if (ManagerList[i] != nullptr)
		{
			delete ManagerList[i];
			ManagerList[i] = nullptr;
		}
	}
	delete[] ManagerList;
	ManagerList = nullptr;
}

void PixelEngine::QuitWindow()
{
	if(ManagerList != nullptr)
	{
		WindowPlatform* win = static_cast<WindowPlatform*>(ManagerList[0]);
		win->QuitWindow();
	}
}

float PixelEngine::GetDeltaTime()
{
	if (timeManager != nullptr) 
	{
		return timeManager->GetDeltaTime();
	}
	return 0;
}

double PixelEngine::GetTotalTime()
{
	if (timeManager != nullptr)
	{
		return timeManager->GetTotalTime();
	}
	return 0;
}

int PixelEngine::GetFPS()
{
	if (timeManager != nullptr)
	{
		return timeManager->GetFPS();
	}
	return 0;
}
