#include "pch.h"
#include "PixelEngine.h"
#include "PixelGraphicsAPI.h"
#include "Core/Module.h"
#include "Scene.h"
#include <iostream>
#include <Windows.h>
#include "TimeManager.h"

#include "ObjectManager.h"
#include "KeyInputManager.h"
#include "LuaManager.h"
#include "FunctionManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "CollisionManager.h"
#include "BindManager.h"
#include "JsonManager.h"
#include "GenerateManager.h"
#include "SPointer.h"
#include "PixelMetaAPI.h"
#include "EventManager.h"

void PixelEngine::Initialize(HWND hWnd, int width, int height)
{
	BindFactory<KeyInputManager>();
	BindFactory<TimeManager>();
	BindFactory<ObjectManager>();
	BindFactory<BindManager>();
	BindFactory<LuaManager>();
	BindFactory<SceneManager>();
	BindFactory<FunctionManager>();
	BindFactory<ResourceManager>();
	BindFactory<CollisionManager>();
	BindFactory<JsonManager>();
	BindFactory<GenerateManager>();
	BindFactory<EventManager>();

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
	PixelGraphicsRendering(0.25f, 0.25f, 0.25f, 1.0f);
}

bool PixelEngine::RunningCheck()
{
	return true;
}

void PixelEngine::ReleaseShared()
{
	for (auto& k : factoryMap)
	{
		k.second->Release();
		delete k.second;
		k.second = nullptr;
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
	//auto sceneManager = GetFactory<SceneManager>();
	//sceneManager->Clear();
	//auto resource = GetFactory<ResourceManager>();
	//resource->Clear();
	//auto luaManager = GetFactory<LuaManager>();
	//luaManager->Clear();
	for (auto& k : factoryMap)
	{
		k.second->Clear();
	}
	GraphicsClear();
	Import("./Asset/main.lua");
}

void PixelEngine::QuitWindow()
{
	
}

SPointer<GameObject> PixelEngine::CreateGameObject(std::string name)
{
	auto factory = GetFactory<ObjectManager>();
	return factory->Create(name);
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


