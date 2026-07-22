#pragma once
#include "PixelEngineDLL.h"
struct PVector3
{
	float x;
	float y;
	float z;
};

extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Normalize(PVector3 vector3);
extern "C" PIXEL_ENGINEDLL float PVector3_Length(PVector3 vector3);
extern "C" PIXEL_ENGINEDLL float PVector3_Distance(PVector3 from, PVector3 to);
extern "C" PIXEL_ENGINEDLL float PVector3_Dot(PVector3 lhs, PVector3 rhs);
extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Cross(PVector3 lhs, PVector3 rhs);
extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Direction(PVector3 from, PVector3 to);
extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Lerp(PVector3 from, PVector3 to, float amount);
extern "C" PIXEL_ENGINEDLL bool PVector3_ApproximatelyEquals(PVector3 lhs, PVector3 rhs, float epsilon);
extern "C" PIXEL_ENGINEDLL PVector3 PVector3_Create(float x, float y, float z);
