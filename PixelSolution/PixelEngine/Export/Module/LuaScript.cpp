#include "pch.h"
#include "LuaScript.h"
#include "sol.hpp"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "LuaModuleInfo.h"
#include "LuaManager.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Log.h"
extern PixelEngine* Engine;
LuaManager* LuaScript::lua = nullptr;
LuaScript::LuaScript()
{
    if (lua == nullptr)
    {
        lua = Engine->GetFactory<LuaManager>();
    }
}

LuaScript::~LuaScript()
{
    //instance = nullptr;
}

void LuaScript::Awake()
{
    if (awake.valid())
    {
    	auto result = awake(instance);
    	if (!result.valid())
    	{
    		sol::error err = result;
    		std::string what = err.what();
    		PixelLog::Error("--- LUA AWAKE ERROR ---");
    		PixelLog::Error(what);
    		PixelLog::Error("-----------------------");
    	}
    }

}

void LuaScript::Start()
{
    if (start.valid())
    {
    	auto result = start(instance);
    	if (!result.valid())
    	{
    		sol::error err = result;
    		std::string what = err.what();
    		PixelLog::Error("--- LUA SRART ERROR ---");
    		PixelLog::Error(what);
    		PixelLog::Error("-----------------------");
    	}
    }
}

void LuaScript::Update()
{
    if (update.valid())
    {
        auto result = update(instance, GetDeltaTime());
    }
}


void LuaScript::Reload()
{
    if (update.valid())
    {
        auto result = update(instance, GetDeltaTime());
        if (!result.valid())
        {
            sol::error err = result;
            std::string what = err.what();
            PixelLog::Error("--- LUA SRART ERROR ---");
            PixelLog::Error(what);
            PixelLog::Error("-----------------------");
        }
    }
}

void LuaScript::Register(std::string fileName)
{
    auto luaInfo = lua->GetModuleLua(fileName);
    if (luaInfo == nullptr) 
    {
        PixelLog::Error("Not Find Lua File :" + fileName);
        return;
    }
    luaFileName = fileName;
    instance = luaInfo->Create();
    
    awake = instance["Awake"];
    start = instance["Start"];
    update = instance["Update"];
    
    instance["gameObject"] = targetObject;
    instance["transform"] = transform;
}
