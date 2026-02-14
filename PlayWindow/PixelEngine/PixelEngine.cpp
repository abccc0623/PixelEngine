#include "PixelEngine.h"
#include "PixelGraphicsAPI.h"
#include "Module.h"
#include "Scene.h"
#include <iostream>
#include <Windows.h>
#include "TimeManager.h"

#include "FactoryManager.h"
#include "KeyInputManager.h"
#include "LuaManager.h"
#include "FunctionManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ResourceManager.h"

#include "ModuleTypeList.h"
extern std::unordered_map<std::string, std::function<void(GameObject*)>> moduleFactories;
void PixelEngine::Initialize(HWND hWnd, int width, int height)
{
	ManagerList = std::vector<EngineManager*>();
	keyInputManager = new KeyInputManager();
	timeManager = new TimeManager();
	factoryManager = new FactoryManager();
	luaManager = new LuaManager();
	sceneManager = new SceneManager();
	functionManager = new FunctionManager();
	resourceManager = new ResourceManager();

	BindFactory<KeyInputManager>();
	BindFactory<TimeManager>();
	BindFactory<FactoryManager>();
	BindFactory<LuaManager>();
	BindFactory<SceneManager>();
	BindFactory<FunctionManager>();
	BindFactory<ResourceManager>();

	auto k =  GetFactory<TimeManager>();



	ManagerList.push_back(keyInputManager);
	ManagerList.push_back(timeManager);
	ManagerList.push_back(factoryManager);
	ManagerList.push_back(functionManager);
	ManagerList.push_back(luaManager);
	ManagerList.push_back(sceneManager);
	ManagerList.push_back(resourceManager);

	PixelGraphicsInitialize(hWnd, width, height);

	for (auto& k : factoryMap)
	{
		k.second->Initialize();
	}
	ModuleRegister();
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
	for (auto& k : factoryMap)
	{
		k.second->Update();
	}
	PixelGraphicsRendering(0.25f, 0.25f, 0.25f, 1.0f);
}

bool PixelEngine::RunningCheck()
{
	return (ManagerList.size() == 0) ? false : true;
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

int PixelEngine::GetMousePosition_X()
{
	return keyInputManager->MousePosition_X();
}

int PixelEngine::GetMousePosition_Y()
{
	return keyInputManager->MousePosition_Y();
}

void PixelEngine::Release()
{
	for (int i = 0; i < ManagerList.size(); i++)
	{
		ManagerList[i]->Release();
		if (ManagerList[i] != nullptr)
		{
			delete ManagerList[i];
			ManagerList[i] = nullptr;
		}
	}
	ManagerList.clear();
}

void PixelEngine::QuitWindow()
{
	
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

bool PixelEngine::LoadLuaScript(const std::string& path)
{
	if (luaManager != nullptr)
	{
		return luaManager->LoadLuaScript(path);
	}
	return false;
}

bool PixelEngine::CreateLuaAPIPath(const std::string& path)
{
	if (luaManager != nullptr)
	{
		return luaManager->CreateLuaAPIPath(path);
	}
	return false;
}

GameObject* PixelEngine::CreateGameObject()
{
	if (factoryManager == nullptr) 
	{
		std::cout << "Not Find FactoryManager" << std::endl;
		return nullptr;
	}
	return factoryManager->Get();
}

Scene* PixelEngine::CreateScene(std::string name)
{
	if (sceneManager != nullptr)
	{
		sceneManager->CreateScene(name);
	}
	return nullptr;
}

ObjectID PixelEngine::Load(RESOURCE_TYPE type, const std::string& path)
{
	if (resourceManager != nullptr)
	{
		resourceManager->Load(type,path);
	}
	return 0;
}

ObjectID PixelEngine::GetResourceID(RESOURCE_TYPE type, const std::string& path)
{
	if (resourceManager != nullptr)
	{
		return resourceManager->Get(type, path);
	}
	return ObjectID();
}

sol::state* PixelEngine::GetLua()
{
	if (luaManager != nullptr) {return luaManager->GetLua();}
	return nullptr;
}

void PixelEngine::RegisterFunction(GameObject* obj, Module* module, int type)
{
 	if (functionManager == nullptr)
	{
		std::cout << "Not Find functionManager" << std::endl;
		return;
	}
	functionManager->RegisterFunction(obj, module, type);
}


