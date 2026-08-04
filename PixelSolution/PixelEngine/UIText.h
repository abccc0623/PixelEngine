#pragma once
#include "PixelEngineDLL.h"
#include <cstdint>
#include <string>

struct UITextData
{
	unsigned int thisID;
	std::string text;
	std::uint16_t fontKey = 0;
	float scale = 1.0f;
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

extern "C" PIXEL_ENGINEDLL UITextData* UIText_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL UITextData* UIText_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool UIText_Has(unsigned int id);
extern "C" PIXEL_ENGINEDLL void UIText_SetText(unsigned int id, const char* text);
extern "C" PIXEL_ENGINEDLL void UIText_SetTextSize(unsigned int id, float scale);
extern "C" PIXEL_ENGINEDLL void UIText_SetFont(unsigned int id, const char* name);
extern "C" PIXEL_ENGINEDLL void UIText_SetColor(unsigned int id, float r, float g, float b, float a);
