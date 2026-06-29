#pragma once
#include "PixelEngineDLL.h"

extern "C" PIXEL_ENGINEDLL void Debug_Log(const char* log);
extern "C" PIXEL_ENGINEDLL void Debug_LogError(const char* log);
extern "C" PIXEL_ENGINEDLL void Debug_LogWarning(const char* log);

extern "C" PIXEL_ENGINEDLL void Debug_Line(float startX, float startY, float startZ, float endX, float endY, float endZ, float r, float g, float b);
extern "C" PIXEL_ENGINEDLL void Debug_Box2D(float x, float y, float z, float width, float height, float r, float g, float b);
extern "C" PIXEL_ENGINEDLL void Debug_Sphere2D(float x, float y, float z, float radius, float r, float g, float b);
