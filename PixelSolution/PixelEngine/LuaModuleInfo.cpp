#include "pch.h"
#include "LuaModuleInfo.h"
#include "PixelEngineAPI.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Log.h"
LuaModuleInfo::LuaModuleInfo(sol::table table)
{
	this->tabel = table;
}
LuaModuleInfo::~LuaModuleInfo()
{

}

void LuaModuleInfo::Reload()
{

}

void LuaModuleInfo::Awake()
{
	//sol::state_view lua(tabel.lua_state());
	//sol::table prototypeSelf = tabel["self"];
	//
	//instance = lua.create_table();
	//sol::table mt = lua.create_table();
	//mt["__index"] = prototypeSelf;
	//instance[sol::metatable_key] = mt;
	//
	//instance["gameObject"] = targetObject;
	//instance["transform"] = transform;
	//
	//luaAwake = prototypeSelf["Awake"];
	//luaStart = prototypeSelf["Start"];
	//luaUpdate = prototypeSelf["Update"];
	//
	//if (luaAwake.valid())
	//{
	//	auto result = luaAwake(instance);
	//	if (!result.valid())
	//	{
	//		sol::error err = result;
	//		std::string what = err.what();
	//		PixelLog::Error("--- LUA AWAKE ERROR ---");
	//		PixelLog::Error(what);
	//		PixelLog::Error("-----------------------");
	//	}
	//}
}

void LuaModuleInfo::Start()
{
	//if (luaStart.valid())
	//{
	//	auto result = luaStart(instance);
	//	if (!result.valid())
	//	{
	//		sol::error err = result;
	//		std::string what = err.what();
	//		PixelLog::Error("--- LUA SRART ERROR ---");
	//		PixelLog::Error(what);
	//		PixelLog::Error("-----------------------");
	//	}
	//}
}

void LuaModuleInfo::Update()
{
	//if (luaUpdate.valid())
	//{
	//    auto result = luaUpdate(instance, GetDeltaTime());
	//}
}

sol::table LuaModuleInfo::Create()
{
	sol::state_view lua(tabel.lua_state());

	// ? 기존: tabel["self"]를 찾음 (전역 오염의 원인)
	// sol::table prototypeSelf = tabel["self"]; 

	// ? 수정: tabel 자체가 return된 'Script' 테이블(원본)입니다!
	sol::table prototypeSelf = tabel;

	auto instance = lua.create_table();
	sol::table mt = lua.create_table();
	mt["__index"] = prototypeSelf; // 순수한 원본을 연결
	instance[sol::metatable_key] = mt;

	return instance;
}

//void LuaModuleInfo::Set(Transform* transform)
//{
//	this->transform = transform;
//}

//void LuaModuleInfo::Set(GameObject* targetObject)
//{
//	this->targetObject = targetObject;
//}

