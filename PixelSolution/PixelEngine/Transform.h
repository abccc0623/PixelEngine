#pragma once
#include "PixelMath.h"
#include "PVector3.h"
#include "PixelEngineDLL.h"
struct TransformData
{
	PVector3 position{ 0.0f, 0.0f, 0.0f };
	unsigned int thisID;

	PVector3 rotation{ 0.0f, 0.0f, 0.0f };
	unsigned int bitmask = 1;

	PVector3 scale{ 1.0f, 1.0f, 1.0f };
	unsigned int  unused2;
};

struct WorldData
{
	Pixel::Matrix4x4 world;
};

extern "C" PIXEL_ENGINEDLL TransformData* Transform_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL TransformData* Transform_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool Transform_Has(unsigned int id);
