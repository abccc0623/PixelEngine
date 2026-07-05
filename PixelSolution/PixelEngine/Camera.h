#pragma once
#include "PixelGraphicsAPI.h"
#include "PixelEngineDLL.h"

struct CameraData
{

};

extern "C" PIXEL_ENGINEDLL CameraData* Camera_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL CameraData* Camera_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool Camera_Has(unsigned int id);