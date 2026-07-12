#pragma once
#include "PixelGraphicsAPI.h"
#include "PixelEngineDLL.h"
struct Renderer2DData
{
	unsigned int thisID;
	float TilingX = 1.0f;
	float TilingY = 1.0f;
	float OffsetX = 1.0f;
	float OffsetY = 1.0f;
};

struct GraphicsData
{
	RenderingData renderingData;
};

extern "C" PIXEL_ENGINEDLL Renderer2DData* Renderer2D_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL Renderer2DData* Renderer2D_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool Renderer2D_Has(unsigned int id);
extern "C" PIXEL_ENGINEDLL void Renderer2D_SetTexture(unsigned int id, const char* name);


