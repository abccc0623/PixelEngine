#include "pch.h"
#include "LuaScript.h"
#include "sol.hpp"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "LuaClassInfo.h"
#include "LuaManager.h"
#include "Log.h"
extern PixelEngine* Engine;
LuaManager* LuaScript::lua = nullptr;
LuaScript::LuaScript()
{
    if (lua == nullptr)
    {
        lua = Engine->GetFactory<LuaManager>();
    }
    info = nullptr;
}

LuaScript::~LuaScript()
{
    info = nullptr;
}

void LuaScript::Awake()
{
    if (info != nullptr)
    {
        info->Awake();
    }
}

void LuaScript::Start()
{
    if (info != nullptr)
    {
        info->Start();
    }
}

void LuaScript::Update()
{
    if (info != nullptr)
    {
        info->Update();
    }
}


void LuaScript::Reload()
{
    if (info != nullptr)
    {
        info->Reload();
    }
}

void LuaScript::Register(std::string fileName)
{
    auto luaInfo = lua->GetLua(fileName);
    if (luaInfo == nullptr) 
    {
        Log::Error("Not Find Lua File :" + fileName);
        return;
    }
    info = luaInfo;
    luaFileName = fileName;
}
