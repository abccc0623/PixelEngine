#pragma once
#include "PixelEngineDLL.h"

struct UIImageData
{
	unsigned int thisID;
	int order = 0;
};

extern "C" PIXEL_ENGINEDLL UIImageData* UIImage_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL UIImageData* UIImage_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool UIImage_Has(unsigned int id);
extern "C" PIXEL_ENGINEDLL void UIImage_SetTexture(unsigned int id, const char* name);