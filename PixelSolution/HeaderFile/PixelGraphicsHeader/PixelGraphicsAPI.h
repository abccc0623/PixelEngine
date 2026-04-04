#pragma once
#include "PixelGraphicsDLL.h"
#include "RenderringData.h"
#include <windows.h>

PICNIC_GRAPHICS_DLL bool PixelGraphicsInitialize(HWND hwnd, int Width, int Height);
PICNIC_GRAPHICS_DLL void PixelGraphicsRelease();
PICNIC_GRAPHICS_DLL void PixelGraphicsRendering();
PICNIC_GRAPHICS_DLL void PixelGraphicsResize(int Width, int Height);
PICNIC_GRAPHICS_DLL void PixelGraphicsBackgroundColor(int R, int G, int B);

PICNIC_GRAPHICS_DLL RenderingData* GetRenderingData();
PICNIC_GRAPHICS_DLL void DeleteRenderingData(RenderingData* mData);
PICNIC_GRAPHICS_DLL void ChangeRenderingData(RenderingData* mData);

PICNIC_GRAPHICS_DLL ObjectID LoadGraphicsTexture(const char* filePath);
PICNIC_GRAPHICS_DLL void GraphicsClear();

