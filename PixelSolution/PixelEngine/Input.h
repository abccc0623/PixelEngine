#pragma once
#include "PixelEngineDLL.h"
#include "Type/GlobalEnum.h"
extern "C" PIXEL_ENGINEDLL bool Input_Key(Keycode key);
extern "C" PIXEL_ENGINEDLL bool Input_KeyDown(Keycode key);
extern "C" PIXEL_ENGINEDLL bool Input_KeyUp(Keycode key);
extern "C" PIXEL_ENGINEDLL int Input_GetMousePositionX();
extern "C" PIXEL_ENGINEDLL int Input_GetMousePositionY();

