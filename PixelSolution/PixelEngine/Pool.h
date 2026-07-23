#pragma once
#include "PixelEngineDLL.h"
extern "C" PIXEL_ENGINEDLL unsigned int Pool_Active(const char* scriptName);
extern "C" PIXEL_ENGINEDLL void Pool_SetExpansionSize(const char* scriptName, int CreateCount);
extern "C" PIXEL_ENGINEDLL void Pool_Disable(const char* scriptName, unsigned int id);
extern "C" PIXEL_ENGINEDLL void Pool_Clear(const char* scriptName);
extern "C" PIXEL_ENGINEDLL int Pool_GetActiveCount(const char* scriptName);
extern "C" PIXEL_ENGINEDLL unsigned int Pool_GetActiveID(const char* scriptName, int index);
