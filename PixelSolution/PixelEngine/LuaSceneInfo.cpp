#include "pch.h"
#include "LuaSceneInfo.h"


LuaSceneInfo::LuaSceneInfo(sol::table table)
{
	this->table = table;
}

LuaSceneInfo::LuaSceneInfo(sol::protected_function factory)
	: factory(std::move(factory))
{
}

std::unique_ptr<LuaSceneInfo> LuaSceneInfo::Create()
{
	auto result = factory();
	if (!result.valid())
	{
		sol::error error = result;
		PixelLog::Error("Lua scene creation failed: " + std::string(error.what()));
		return nullptr;
	}
	if (result.return_count() == 0 || !result[0].is<sol::table>())
	{
		PixelLog::Error("Lua scene must return a table.");
		return nullptr;
	}
	return std::make_unique<LuaSceneInfo>(result[0].as<sol::table>());
}

LuaSceneInfo::~LuaSceneInfo()
{

}

void LuaSceneInfo::Start()
{
	sol::protected_function startFunc = table["Start"];
	if (startFunc.valid())
	{
		auto result = startFunc(table);
		if (!result.valid())
		{
			sol::error err = result;
			std::string errorMsg = err.what();
			PixelLog::Error(errorMsg.c_str());
		}
	}
}

void LuaSceneInfo::Update()
{
	sol::protected_function updateFunc = table["Update"];
	if (updateFunc.valid())
	{
		auto result = updateFunc(table);
		if (!result.valid())
		{
			sol::error err = result;
			std::string errorMsg = err.what();
			PixelLog::Error(errorMsg.c_str());
		}
	}
}

void LuaSceneInfo::Release()
{
	//sol::protected_function releaseFunc = table["Update"];
	//if (releaseFunc.valid())
	//{
	//	auto result = releaseFunc(table);
	//	if (!result.valid())
	//	{
	//		sol::error err = result;
	//		std::string errorMsg = err.what();
	//		PixelLog::Error(errorMsg.c_str());
	//	}
	//}
}
