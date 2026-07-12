#pragma once
#include "PixelEngineDLL.h"
struct PVector2
{
	float x;
	float y;
};

extern "C" PIXEL_ENGINEDLL PVector2 PVector2_Create(float x, float y);