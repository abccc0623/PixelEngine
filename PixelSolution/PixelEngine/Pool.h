#pragma once
#include "PixelEngineDLL.h"
extern "C" PIXEL_ENGINEDLL unsigned int Pool_Active(const char* scriptName);
extern "C" PIXEL_ENGINEDLL void Pool_Disable(const char* scriptName, unsigned int id);
extern "C" PIXEL_ENGINEDLL void Pool_Clear(const char* scriptName);
