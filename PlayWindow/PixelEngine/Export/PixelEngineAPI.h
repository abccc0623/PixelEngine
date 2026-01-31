#pragma once
#include "PixelEngineDLL.h"
extern "C" PIXEL_ENGINEDLL bool EngineInitialize();							
extern "C" PIXEL_ENGINEDLL void UpdateEngine();
extern "C" PIXEL_ENGINEDLL void ReleaseEngine();							
extern "C" PIXEL_ENGINEDLL void QuitWindow();

//KeyInput
extern "C" PIXEL_ENGINEDLL bool GetKeyDown(byte number);		
extern "C" PIXEL_ENGINEDLL bool GetKeyUp(byte number);			
extern "C" PIXEL_ENGINEDLL bool GetKey(byte number);

//Time
extern "C" PIXEL_ENGINEDLL float GetDeltaTime();
extern "C" PIXEL_ENGINEDLL double GetTotalTime();
extern "C" PIXEL_ENGINEDLL int GetFPS();
