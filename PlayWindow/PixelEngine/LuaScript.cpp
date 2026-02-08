#include "LuaScript.h"
#include <filesystem>
#include "sol.hpp"
#include "PixelEngine.h"
#include "GameObject.h"
#include <string>
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
        auto result = luaUpdate(selfTable);
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

void LuaScript::RegisterFile(std::string fileName)
{
    auto target = Engine->GetLua();
    sol::protected_function_result result = target->do_file(fileName);
    path = fileName;
    if (result.valid())
    {
        selfTable = result;
        selfTable["this"] = this->targetObject;
        luaStart = selfTable["Awake"];
        luaStart = selfTable["Start"];
        luaUpdate = selfTable["Update"];
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
