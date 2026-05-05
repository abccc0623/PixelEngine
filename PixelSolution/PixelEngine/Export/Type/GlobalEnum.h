#pragma once
#include "PixelMetaAPI.h"
enum class MODULE_TYPE
{
	NONE,
	Transform,
	LuaScript,
	Camera,
	DebugCamera,
	Renderer2D,
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


enum class EventType : int
{
	KeyUp,
	KeyDown,
	CollisionIn,
	CollisionOut,
};

enum class ColliderMotionType : int 
{
	Static,
	Kinematic,
	Dynamic,
};

enum class ColliderType : int
{
	Box2D,
	Circle2D
};




