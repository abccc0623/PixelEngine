#pragma once
#include "PixelEngineDLL.h"

extern "C" PIXEL_ENGINEDLL bool Event_Bind(unsigned int id, const char* EventName, const char* bindFunctionName);
extern "C" PIXEL_ENGINEDLL bool Event_Call(const char* EventName);