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
	PVector3 localPosition{ 0, 0, 0 };
	PVector3 localRotation{ 0, 0, 0 };
	PVector3 localScale{ 1, 1, 1 };

	Pixel::Matrix4x4 world;
	Pixel::Matrix4x4 local;
};

extern "C" PIXEL_ENGINEDLL TransformData* Transform_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL TransformData* Transform_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool Transform_Has(unsigned int id);
extern "C" PIXEL_ENGINEDLL void Transform_SetParent(unsigned int id, unsigned int ParentID);
extern "C" PIXEL_ENGINEDLL void Transform_SetChild(unsigned int id, unsigned int ChildID);
extern "C" PIXEL_ENGINEDLL void Transform_SetLocalPosition(unsigned int id, PVector3 position);
extern "C" PIXEL_ENGINEDLL void Transform_SetLocalRotation(unsigned int id, PVector3 rotation);
extern "C" PIXEL_ENGINEDLL void Transform_SetLocalScale(unsigned int id, PVector3 scale);



