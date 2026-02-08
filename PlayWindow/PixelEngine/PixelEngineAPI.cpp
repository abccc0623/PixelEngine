#include "PixelEngineAPI.h" 
#include "PixelEngine.h" 
#include "WindowPlatform.h"
#include "ResourceManager.h"
#include <string>

PixelEngine* Engine = nullptr;
WindowPlatform* window = nullptr;
bool EngineInitialize(HWND hWnd, int width, int height)
{
	Engine = new PixelEngine();
	Engine->Initialize(hWnd, width, height);
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
	window->Release();
	delete window;
	window = nullptr;
}

void QuitWindow()
{
	if (window != nullptr)
	{
		//Engine->QuitWindow();
	}
}

bool GetKeyDown(char number)
{
	if (Engine != nullptr) return Engine->GetKeyDown(number);
	return false;
}

bool GetKeyUp(char number)
{
	if (Engine != nullptr) return Engine->GetKeyUp(number);
	return false;
}

bool GetKey(char number)
{
	if (Engine != nullptr) return Engine->GetKey(number);
	return false;
}

int GetMousePosition_X()
{
	if (Engine != nullptr) return Engine->GetMousePosition_X();
	return -1;
}

int GetMousePosition_Y()
{
	if (Engine != nullptr) return Engine->GetMousePosition_Y();
	return -1;
}

float GetDeltaTime()
{
	if (Engine != nullptr) return Engine->GetDeltaTime();
	return false;
}

double GetTotalTime()
{
	if  (Engine != nullptr) return Engine->GetTotalTime();
	return false;
}

int GetFPS()
{
	if (Engine != nullptr) return Engine->GetFPS();
	return false;
}

bool LoadLuaScript(const std::string& path)
{
	if (Engine != nullptr) return Engine->LoadLuaScript(path);
	return false;
}

bool LoadTexture(const std::string& path)
{
	if (Engine != nullptr) return Engine->Load(RESOURCE_TYPE::TEXTURE,path);
	return true;
}

bool CreateLuaAPIPath(const std::string& path)
{
	if (Engine != nullptr) return Engine->CreateLuaAPIPath(path);
	return false;
}

						