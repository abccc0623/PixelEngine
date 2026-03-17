#include "pch.h"
#include "PixelEngineAPI.h" 
#include "PixelEngine.h" 

#include <windows.h>
#include "ObjectManager.h"
#include "KeyInputManager.h"
#include "LuaManager.h"
#include "GenerateManager.h"
#include "FunctionManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "TimeManager.h"
#include "Core/GameObject.h"
#include "JsonManager.h"
#include "PixelMetaAPI.h"
#include "Log.h"


PixelEngine* Engine = nullptr;
LogCallbackFunc g_logCallback = nullptr;
SceneChangeCallbackFunc g_SceneObjectChangeCallBack = nullptr;

bool EngineInitialize(PixelWindowHandle hWnd, int width, int height)
{
	Engine = new PixelEngine();
	Engine->Initialize((HWND)hWnd, width, height);
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
	ReleaseMetaType();
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

bool LoadLuaFile(const char* path)
{
	std::string strPath(path);
	if (Engine != nullptr)
	{
		auto lua = Engine->GetFactory<LuaManager>();
		return lua->Load(strPath);
	}
	return false;
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
	return false;
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

Module* AddModule(GameObject* target, PClass* moduleClass)
{
	return target->AddModule(moduleClass);
}

Module* AddModuleByString(GameObject* target, const char* name)
{
	std::string Str(name);
	PClass* targetClass = GetMetaClass(name);
	if (target != nullptr)
	{
		return target->AddModule(targetClass);
	}
	else
	{
		Log::Error("Not Find Type :"+ Str);
	}
	return nullptr;
}

Module* GetModule(GameObject* target, PClass* moduleClass)
{
	return target->GetModule(moduleClass);
}

Module* GetModuleByString(GameObject* target, const char* name)
{
	return nullptr;
}

#pragma region MetaType
PClass* GetMetaClass(const char* className)
{
	std::string targetName(className);
	return GetClass(targetName);
}
const char* GetMemberName(PClass* targetClass, int index)
{
	return GetClassMemberName(targetClass, index).c_str();
}
const char* GetMemberType(PClass* targetClass, int index)
{
	return GetClassMemberType(targetClass, index).c_str();
}
const char* GetMethodName(PClass* targetClass, int index)
{
	return GetClassMethodName(targetClass,index).c_str();
}
const char* GetMethodReturnType(PClass* targetClass, int index)
{
	return GetClassMethodReturnType(targetClass, index).c_str();
}
const char* GetMethodGetPropertyType(PClass* targetClass, int index, int propertyIndex)
{
	return GetClassMethodGetPropertyType(targetClass, index, propertyIndex).c_str();
}
int GetMethodPropertyCount(PClass* targetClass, int index)
{
	return GetClassMethodPropertyCount(targetClass, index);
}
void GenerateLuaBindCode(const char* outPath)
{
	if (Engine != nullptr)
	{
		GenerateManager* Generate = Engine->GetFactory<GenerateManager>();
		Generate->CreateLuaBindCode(outPath);
	}
}
int GetMemberCount(PClass* targetClass)
{
	return GetClassMemberCount(targetClass);
}
int GetMethodCount(PClass* targetClass)
{
	return GetClassMethodCount(targetClass);
}
#pragma endregion

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

void LoadScene(const char* sceneName)
{
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		auto path = std::string(sceneName);
		JsonManager::Load(path);
	}
}

void RegisterLogCallback(LogCallbackFunc callback)
{
	g_logCallback = callback;
}

void DispatchNativeLog(const char* msg, int level)
{
	if (g_logCallback)
	{
		g_logCallback(msg, level);
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

GameObject** GetAllSceneObjects(int* outCount)
{
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		return scene->GetAllSceneObjects(outCount);
	}
	return 0;
}

void  RegisterSceneObjectChange(SceneChangeCallbackFunc callBack)
{
	g_SceneObjectChangeCallBack = callBack;
}

						