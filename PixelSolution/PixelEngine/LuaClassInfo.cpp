#include "pch.h"
#include "LuaClassInfo.h"

LuaClassInfo::LuaClassInfo(sol::table table)
{
    this->tabel = table;
}
LuaClassInfo::~LuaClassInfo()
{

}

void LuaClassInfo::Reload()
{
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

void LuaClassInfo::Awake()
{
    //if (luaAwake.valid())
    //{
    //    auto result = luaAwake(selfTable);
    //    if (!result.valid())
    //    {
    //        sol::error err = result;
    //        std::string what = err.what();
    //        std::cout << "--- LUA AWAKE ERROR ---" << std::endl;
    //        std::cout << what << std::endl;
    //        std::cout << "-----------------------" << std::endl;
    //    }
    //}
}

void LuaClassInfo::Start()
{
    //if (luaStart.valid())
  //{
  //    auto result = luaStart(selfTable);
  //    if (!result.valid())
  //    {
  //        sol::error err = result;
  //        std::string what = err.what();
  //        std::cout << "--- LUA SRART ERROR ---" << std::endl;
  //        std::cout << what << std::endl;
  //        std::cout << "-----------------------" << std::endl;
  //    }
  //}
}

void LuaClassInfo::Update()
{
    //if (luaUpdate.valid())
    //{
    //    auto result = luaUpdate(selfTable, GetDeltaTime());
    //    if (!result.valid())
    //    {
    //        if (updateLog == false)
    //        {
    //            sol::error err = result;
    //            std::string what = err.what();
    //            std::cout << "--- LUA UPDATE ERROR ---" << std::endl;
    //            std::cout << what << std::endl;
    //            std::cout << "-----------------------" << std::endl;
    //            updateLog = true;
    //        }
    //    }
    //}
}

