#include "pch.h"
#include "LuaSceneInfo.h"

LuaSceneInfo::LuaSceneInfo(sol::table table)
{
	this->table = table;
}

LuaSceneInfo::~LuaSceneInfo()
{

}

void LuaSceneInfo::Start()
{
	auto startFunc = table["Start"];
	if (startFunc.valid())
	{
		startFunc();
	}
}

void LuaSceneInfo::Update()
{
	auto updateFunc = table["Update"];
	if (updateFunc.valid())
	{
		updateFunc();
	}
}

void LuaSceneInfo::Release()
{
	auto releaseFunc = table["Release"];
	if (releaseFunc.valid())
	{
		releaseFunc();
	}
}
