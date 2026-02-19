#include "PixelEngineAPI.h" 
#include "PixelEngine.h" 
#include <string>

#include "ObjectManager.h"
#include "KeyInputManager.h"
#include "LuaManager.h"
#include "FunctionManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "TimeManager.h"



PixelEngine* Engine = nullptr;
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
	if (Engine != nullptr) 
	{
		Engine->Release();
	}
}

void QuitWindow()
{
	
}

bool ResizeEngine(int width, int height)
{
	if (Engine != nullptr)
	{
		Engine->Resize(width, height);
	}
	return true;
}

bool GetKeyDown(char number)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->GetKeyDown(number);
	}
	return false;
}

bool GetKeyUp(char number)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->GetKeyUp(number);
	}
	return false;
}

bool GetKey(char number)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->GetKey(number);
	}
	return false;
}

int GetMousePosition_X()
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->MousePosition_X();
	}
	return -1;
}

int GetMousePosition_Y()
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->MousePosition_Y();
	}
	return -1;
}

float GetDeltaTime()
{
	if (Engine != nullptr)
	{
		auto time = Engine->GetFactory<TimeManager>();
		return time->GetDeltaTime();
	}
	return -1;
}

double GetTotalTime()
{
	if (Engine != nullptr)
	{
		auto time = Engine->GetFactory<TimeManager>();
		return time->GetTotalTime();
	}
	return -1;
}

int GetFPS()
{
	if (Engine != nullptr)
	{
		auto time = Engine->GetFactory<TimeManager>();
		return time->GetFPS();
	}
	return -1;
}

bool LoadLuaScript(const std::string& path)
{
	if (Engine != nullptr)
	{
		auto lua = Engine->GetFactory<LuaManager>();
		return lua->LoadLuaScript(path);
	}
}

bool LoadTexture(const std::string& path)
{
	if (Engine != nullptr)
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(TEXTURE, path);
		return true;
	}
}

bool CreateLuaAPIPath(const std::string& path)
{
	if (Engine != nullptr)
	{
		auto lua = Engine->GetFactory<LuaManager>();
		return lua->CreateLuaAPIPath(path);
	}
}

						