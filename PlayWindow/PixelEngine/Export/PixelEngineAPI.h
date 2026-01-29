#pragma once
#include "PixelEngineDLL.h"
#include <windows.h>

extern "C" PIXEL_ENGINEDLL bool EngineInitialize(HWND _g_hWnd);				//GameEngine 시작
extern "C" PIXEL_ENGINEDLL void EndEngine();								//GameEngine 종료
extern "C" PIXEL_ENGINEDLL void UpdateEngine();								//GameEngine 업데이트
extern "C" PIXEL_ENGINEDLL void OnReSize(int X, int Y);						//윈도우 사이즈 재조정
extern "C" PIXEL_ENGINEDLL void GetWindowSize(int* X, int* Y);