#pragma once
#include "PixelEngineDLL.h"

struct UIImageData
{
	unsigned int thisID;
};

extern "C" PIXEL_ENGINEDLL UIImageData* UIImage_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL UIImageData* UIImage_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool UIImage_Has(unsigned int id);
extern "C" PIXEL_ENGINEDLL void UIImage_SetTexture(unsigned int id, const char* name);
extern "C" PIXEL_ENGINEDLL void UIImage_SetOrder(unsigned int id, int order);
extern "C" PIXEL_ENGINEDLL void UIImage_SetColor(unsigned int id, float r, float g, float b, float a);