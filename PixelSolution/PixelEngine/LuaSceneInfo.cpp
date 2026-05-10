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
