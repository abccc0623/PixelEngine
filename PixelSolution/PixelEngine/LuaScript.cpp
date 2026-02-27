#include "pch.h"
#include "LuaScript.h"
#include "sol.hpp"
#include "PixelEngine.h"
#include "GameObject.h"
#include "BindManager.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
extern PixelEngine* Engine;
LuaScript::LuaScript()
{
}

LuaScript::~LuaScript()
{

}

void LuaScript::Awake()
{
    if (luaAwake.valid())
    {
        auto result = luaAwake(selfTable);
        if (!result.valid())
        {
            sol::error err = result;
            std::string what = err.what();
            std::cout << "--- LUA AWAKE ERROR ---" << std::endl;
            std::cout << what << std::endl;
            std::cout << "-----------------------" << std::endl;
        }
    }
}

void LuaScript::Start()
{
    if (luaStart.valid())
    {
        auto result = luaStart(selfTable);
        if (!result.valid())
        {
            sol::error err = result;
            std::string what = err.what();
            std::cout << "--- LUA SRART ERROR ---" << std::endl;
            std::cout << what << std::endl;
            std::cout << "-----------------------" << std::endl;
        }
    }
}

void LuaScript::Update()
{
    if (luaUpdate.valid()) 
    {
        auto result = luaUpdate(selfTable,GetDeltaTime());
        if (!result.valid())
        {
            if (updateLog == false)
            {
                sol::error err = result;
                std::string what = err.what();
                std::cout << "--- LUA UPDATE ERROR ---" << std::endl;
                std::cout << what << std::endl;
                std::cout << "-----------------------" << std::endl;
                updateLog = true;
            }
        }
    }
}

//void LuaScript::OnCollisionEnter2D(WPointer<GameObject> target)
//{
//
//}

void LuaScript::OnCollision2D(WPointer<GameObject> target)
{
    if (OnCollision2DFunc.valid())
    {
        auto result = OnCollision2DFunc(selfTable, target);
        if (!result.valid())
        {
            sol::error err = result;
            std::string what = err.what();
            std::cout << "--- LUA ONCOLLISION ERROR ---" << std::endl;
            std::cout << what << std::endl;
            std::cout << "-----------------------" << std::endl;
        }
    }
}

//void LuaScript::OnCollisionExit2D(WPointer<GameObject> target)
//{
//
//}

void LuaScript::Reload()
{
    if (!path.empty())
    {
        RegisterFile(path);
        Awake();
        Start();
    }
}

void LuaScript::RegisterFile(std::string fileName)
{
    auto target = Engine->GetModuleCall_Lua();
    sol::protected_function_result result = target->do_file(fileName);
    path = fileName;
    if (result.valid())
    {
        auto k = targetObject->GetModule<Transform>();
        selfTable = result;
        selfTable["this"] = this;
        selfTable["obj"]  = targetObject;
        selfTable["transform"] = k.GetPtr();

        luaAwake = selfTable["Awake"];
        luaStart = selfTable["Start"];
        luaUpdate = selfTable["Update"];

        OnCollision2DFunc = selfTable["OnCollision2D"];
        OnCollision2DEnterFunc = selfTable["OnCollision2DEnter"];
        OnCollision2DExitFunc = selfTable["OnCollision2DExit"];
    }
    else 
    {
        sol::error err = result;
        std::string what = err.what();
        std::cout << "--- LUA Register ERROR ---" << std::endl;
        std::cout << what << std::endl;
        std::cout << "-----------------------" << std::endl;
    }
}

std::string LuaScript::RegisterLua()
{
    auto state = GetLuaState();
    state->new_usertype<LuaScript>("LuaScript",
        sol::base_classes, sol::bases<Module, PixelObject>(),
        "RegisterFile", &LuaScript::RegisterFile
    );

    std::string main = "";
    main += BindManager::ExportLuaAPIHeader<LuaScript>();
    main += BindManager::ExportLuaAPIFromFunc("RegisterFile", &LuaScript::RegisterFile, "string");
    return main;
}
