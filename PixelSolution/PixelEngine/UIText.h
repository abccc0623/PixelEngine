#pragma once
#include "PixelEngineDLL.h"

struct UITextData
{
	unsigned int thisID;
};

extern "C" PIXEL_ENGINEDLL UITextData* UIText_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL UITextData* UIText_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool UIText_Has(unsigned int id);
