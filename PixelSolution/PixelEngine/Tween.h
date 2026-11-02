#pragma once
#include "PixelEngineDLL.h"

enum class TweenProperty : int
{
	PositionX,
	PositionY,
	PositionZ,
	RotationX,
	RotationY,
	RotationZ,
	ScaleX,
	ScaleY,
	ScaleZ,
};

enum class TweenEase : int
{
	Linear,
	In,
	Out,
	InOut,
};

extern "C" PIXEL_ENGINEDLL bool Tween_PingPong(unsigned int id, int property, float minValue, float maxValue, float duration, bool useUnscaledTime, int ease = 0);
extern "C" PIXEL_ENGINEDLL bool Tween_Move(unsigned int id, int property, float startValue, float endValue, float duration, bool useUnscaledTime, int ease = 0);
extern "C" PIXEL_ENGINEDLL bool Tween_Pause(unsigned int id, int property);
extern "C" PIXEL_ENGINEDLL bool Tween_Resume(unsigned int id, int property);
extern "C" PIXEL_ENGINEDLL bool Tween_Kill(unsigned int id, int property);
