#include "pch.h"
#include "LuaScript.h"
#include "sol.hpp"
#include "PixelEngine.h"
#include "PixelEngineAPI.h"
#include "LuaClassInfo.h"
#include "LuaManager.h"
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
    if (luaInfo != nullptr)
    {
        info = luaInfo;
    }


    //auto target = Engine->GetModuleCall_Lua();
    //sol::protected_function_result result = target->do_file(fileName);
    //path = fileName;
    //if (result.valid())
    //{
    //    auto k = targetObject->GetModule<Transform>();
    //    selfTable = result;
    //    selfTable["this"] = this;
    //    selfTable["obj"]  = targetObject;
    //    selfTable["transform"] = k.GetPtr();
    //
    //    luaAwake = selfTable["Awake"];
    //    luaStart = selfTable["Start"];
    //    luaUpdate = selfTable["Update"];
    //
    //    OnCollision2DFunc = selfTable["OnCollision2D"];
    //    OnCollision2DEnterFunc = selfTable["OnCollision2DEnter"];
    //    OnCollision2DExitFunc = selfTable["OnCollision2DExit"];
    //}
    //else 
    //{
    //    sol::error err = result;
    //    std::string what = err.what();
    //    std::cout << "--- LUA Register ERROR ---" << std::endl;
    //    std::cout << what << std::endl;
    //    std::cout << "-----------------------" << std::endl;
    //}
}

//void LuaScript::Register(std::string fileName)
//{
    //auto state = GetLuaState();
    //state->new_usertype<LuaScript>("LuaScript",
    //    sol::base_classes, sol::bases<Module, PixelObject>(),
    //    "RegisterFile", &LuaScript::RegisterFile
    //);
    //
    //std::string main = "";
    //main += BindManager::ExportLuaAPIHeader<LuaScript>();
    //main += BindManager::ExportLuaAPIFromFunc("RegisterFile", &LuaScript::RegisterFile, "string");
//}
