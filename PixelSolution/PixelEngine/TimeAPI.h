#pragma once
#include "PixelEngineDLL.h"

extern "C" PIXEL_ENGINEDLL float Time_GetDeltaTime();
extern "C" PIXEL_ENGINEDLL float Time_GetUnscaledDeltaTime();
extern "C" PIXEL_ENGINEDLL double Time_GetTotalTime();
extern "C" PIXEL_ENGINEDLL int Time_GetFPS();
extern "C" PIXEL_ENGINEDLL void Time_SetPaused(bool paused);
extern "C" PIXEL_ENGINEDLL bool Time_IsPaused();
