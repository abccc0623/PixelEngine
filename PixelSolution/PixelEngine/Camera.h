#pragma once
#include "PixelGraphicsAPI.h"
#include "PixelEngineDLL.h"

struct CameraData
{
	unsigned int thisID;
	float FovY = 0.3f * 3.1415926535f;
	float NearZ = 0.1f;
	float FarZ = 4000.0f;
	float ZoomLevel = 1.0f;
};

extern "C" PIXEL_ENGINEDLL CameraData* Camera_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL CameraData* Camera_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool Camera_Has(unsigned int id);