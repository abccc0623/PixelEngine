#pragma once
enum class MODULE_TYPE
{
	NONE,
	Transform,
	LuaScript,
	Camera,
	DebugCamera,
};

enum class MODULE_FUNC : int
{
	//한번 호출되는 함수
	AWAKE,
	START,

	//매프레임 호출되는 함수
	UPDATE,
	MATRIX,
	PHYSICS,
	LAST,

	//특정 조건후 호출되는 함수
};