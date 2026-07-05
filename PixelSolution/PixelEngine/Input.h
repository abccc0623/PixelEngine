#pragma once
#include "PixelEngineDLL.h"
extern "C" PIXEL_ENGINEDLL bool Input_Key(char number);
extern "C" PIXEL_ENGINEDLL bool Input_KeyDown(char number);
extern "C" PIXEL_ENGINEDLL bool Input_KeyUp(char number);
extern "C" PIXEL_ENGINEDLL int Input_GetMousePositionX();
extern "C" PIXEL_ENGINEDLL int Input_GetMousePositionY();

