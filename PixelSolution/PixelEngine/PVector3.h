#pragma once
#include "PixelEngineDLL.h"
struct PVector3
{
	float x;
	float y;
	float z;
};

extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Normalize(PVector3 vector3);
extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Create(float x, float y, float z);
