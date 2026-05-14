#include "pch.h"
#include "LuaModuleInfo.h"
#include "PixelEngineAPI.h"
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

}

void LuaModuleInfo::Start()
{

}

void LuaModuleInfo::Update()
{

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

