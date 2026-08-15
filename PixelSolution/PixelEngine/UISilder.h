#pragma once
#include "PixelEngineDLL.h"
#include "PixelGraphicsAPI.h"

struct UISilderData
{
	unsigned int thisID;
	float minValue;
	float maxValue;
	float value;
	float width;
	float height;
	RenderingData background;
	RenderingData fill;
};

extern "C" PIXEL_ENGINEDLL UISilderData* UISilder_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL UISilderData* UISilder_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool UISilder_Has(unsigned int id);
extern "C" PIXEL_ENGINEDLL void UISilder_SetRange(unsigned int id, float minValue, float maxValue);
extern "C" PIXEL_ENGINEDLL void UISilder_SetValue(unsigned int id, float value);
extern "C" PIXEL_ENGINEDLL void UISilder_SetSize(unsigned int id, float width, float height);
extern "C" PIXEL_ENGINEDLL void UISilder_SetBackgroundColor(unsigned int id, float r, float g, float b, float a);
extern "C" PIXEL_ENGINEDLL void UISilder_SetFillColor(unsigned int id, float r, float g, float b, float a);
extern "C" PIXEL_ENGINEDLL void UISilder_SetBackgroundTexture(unsigned int id, const char* name);
extern "C" PIXEL_ENGINEDLL void UISilder_SetFillTexture(unsigned int id, const char* name);
extern "C" PIXEL_ENGINEDLL void UISilder_SetOrder(unsigned int id, int order);
