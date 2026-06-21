#include "pch.h"
#include "PixelEngine.h"
#include "PixelGraphicsAPI.h"
#include "Core/Module.h"
#include "Scene.h"
#include <iostream>
#include <Windows.h>
#include "TimeManager.h"

#include "KeyInputManager.h"
#include "LuaManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "PhysManager.h"
#include "BindManager.h"
#include "JsonManager.h"
#include "GenerateManager.h"
#include "SPointer.h"
#include "PixelMetaAPI.h"
#include "EventManager.h"
#include "SystemManager.h"
#include "CoroutineManager.h"
#include "EditorManager.h"

#include "Entity.h"

void PixelEngine::Initialize(HWND hWnd, int width, int height, std::string assetPath)
{
	BindFactory<KeyInputManager>();
	BindFactory<TimeManager>();
	BindFactory<BindManager>();
	BindFactory<LuaManager>();
	BindFactory<SceneManager>();
	BindFactory<ResourceManager>();
	BindFactory<PhysManager>();
	BindFactory<JsonManager>();
	BindFactory<GenerateManager>();
	BindFactory<CoroutineManager>();
	BindFactory<EditorManager>();

	EngineRootFolderPath = assetPath;
	if (hWnd != nullptr)
	{
		PixelGraphicsInitialize(hWnd, width, height);
	}

	for (auto& k : factoryMap)
	{
		k.second->Initialize();
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
	for (auto& k : factoryMap)
	{
		k.second->Update();
	}
	PixelGraphicsRendering();

	if (IsClear == true)
	{
		for (auto& k : factoryMap)
		{
			k.second->Clear();
		}
		GraphicsClear();
		Import("./Asset/main.lua");
		IsClear = false;
	}
}

bool PixelEngine::RunningCheck()
{
	return true;
}

std::string PixelEngine::GetEngineRootFolderPath()
{
	if (EngineRootFolderPath.empty())
	{
		return "";
	}
	else
	{
		return EngineRootFolderPath.c_str();
	}
}

void PixelEngine::ReleaseShared()
{
	for (auto it = factoryMap.rbegin(); it != factoryMap.rend(); ++it)
	{
		// it은 reverse_iterator이므로 ->를 통해 요소에 접근합니다.
		if (it->second != nullptr)
		{
			it->second->Release();
			delete it->second;
			it->second = nullptr;
		}
	}
	factoryMap.clear();
	PixelGraphicsRelease();
	MemoryTracker::Get().ReportLeaks();
}

void PixelEngine::Resize(int width, int height)
{
	PixelGraphicsResize(width, height);
}

void PixelEngine::Clear()
{
	IsClear = true;
}

void PixelEngine::CreateEntity(std::string scriptName)
{



}

void PixelEngine::QuitWindow()
{

}

Scene* PixelEngine::CreateScene(std::string name)
{
	//auto scene = GetFactory<SceneManager>();
	//scene->CreateScene(name);
	return nullptr;
}

ObjectID PixelEngine::GetResourceID(RESOURCE_TYPE type, const std::string& path)
{
	auto resource = GetFactory<ResourceManager>();
	return resource->Get(type, path);
}

sol::state* PixelEngine::GetModuleCall_Lua()
{
	auto lua = GetFactory<LuaManager>();
	return nullptr;
}

void PixelEngine::AddFunction(GameObject* obj, Module* module, int type)
{
	//auto func = GetFactory<FunctionManager>();
	//func->AddFunction(obj,MakePixel<Module>(module), type);
}


