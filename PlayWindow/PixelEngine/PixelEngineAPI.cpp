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
#include "SceneManager.h"
#include "GameObject.h"


PixelEngine* Engine = nullptr;
bool EngineInitialize(HWND hWnd, int width, int height)
{
	Engine = new PixelEngine();
	Engine->Initialize(hWnd, width, height);
    return true;
}

void UpdateEngine()
{
	if (Engine != nullptr)
	{
		Engine->Update();
	}
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
		Engine->ReleaseShared();
	}
	delete Engine;
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

bool LoadLuaScript(const char* path)
{
	std::string strPath(path);
	if (Engine != nullptr)
	{
		auto lua = Engine->GetFactory<LuaManager>();
		return lua->LoadLuaScript(strPath);
	}
}

bool LoadTexture(const char* path)
{
	std::string strPath(path);
	if (Engine != nullptr)
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(TEXTURE, strPath);
		return true;
	}
}

bool CreateLuaAPIPath(const char* path)
{
	std::string strPath(path);
	if (Engine != nullptr)
	{
		auto lua = Engine->GetFactory<LuaManager>();
		return lua->CreateLuaAPIPath(strPath);
	}
}

GameObject* CreateGameObject(const char* name)
{
	if (Engine != nullptr)
	{
		auto objManager = Engine->GetFactory<ObjectManager>();
		SPointer<GameObject> target = objManager->Create(name);
		return target.GetPtr();
	}
	return nullptr;
}

bool CreateScene(const char* sceneName)
{
	std::string strPath(sceneName);
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		scene->CreateScene(strPath);
	}
	return true;
}

void SaveScene()
{
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		scene->SaveScene();
	}
}

bool ChangeScene(const char* sceneName)
{
	std::string strPath(sceneName);
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		scene->ChangeScene(strPath);
	}
	return true;
}

						