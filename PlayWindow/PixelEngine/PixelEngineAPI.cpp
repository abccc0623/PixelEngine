#include "PixelEngineAPI.h" 
#include "PixelEngine.h" 
#include "WindowPlatform.h"

PixelEngine* Engine = nullptr;
bool EngineInitialize()
{
	Engine = new PixelEngine();
	Engine->Initialize();
    return true;
}

void UpdateEngine()
{
	Engine->Update();
}

bool RunningEngineCheck()
{
	if (Engine != nullptr)
	{
		return Engine->RunningCheck();
	}
	return false;
}

void ReleaseEngine()
{
	Engine->Release();
	delete Engine;
	Engine = nullptr;
}

void QuitWindow()
{
	if (Engine != nullptr)
	{
		Engine->QuitWindow();
	}
}

bool GetKeyDown(byte number)
{
	if (Engine != nullptr) return Engine->GetKeyDown(number);
	return false;
}

bool GetKeyUp(byte number)
{
	if (Engine != nullptr) return Engine->GetKeyUp(number);
	return false;
}

bool GetKey(byte number)
{
	if (Engine != nullptr) return Engine->GetKey(number);
	return false;
}

float GetDeltaTime()
{
	if (Engine != nullptr) return Engine->GetDeltaTime();
	return false;
}

double GetTotalTime()
{
	if (Engine != nullptr) return Engine->GetTotalTime();
	return false;
}

int GetFPS()
{
	if (Engine != nullptr) return Engine->GetFPS();
	return false;
}
								