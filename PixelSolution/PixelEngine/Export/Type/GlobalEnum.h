#pragma once
#include "PixelMetaAPI.h"
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




