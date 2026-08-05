#pragma once
#include "PixelGraphicsDLL.h"
#include "RenderringData.h"
#include <windows.h>

PICNIC_GRAPHICS_DLL bool PixelGraphicsInitialize(HWND hwnd, int Width, int Height);
PICNIC_GRAPHICS_DLL void PixelGraphicsRelease();
PICNIC_GRAPHICS_DLL void PixelGraphicsRendering();
PICNIC_GRAPHICS_DLL void PixelGraphicsResize(int Width, int Height);
PICNIC_GRAPHICS_DLL void PixelGraphicsBackgroundColor(int R, int G, int B);

PICNIC_GRAPHICS_DLL void SetRenderingData(RenderingData& mData);

PICNIC_GRAPHICS_DLL void DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b);
PICNIC_GRAPHICS_DLL void DrawBox2D(float x, float y, float z, float radius, float r, float g, float b);
PICNIC_GRAPHICS_DLL void DrawCircle2D(float x, float y, float z, float radius, float r, float g, float b);


PICNIC_GRAPHICS_DLL uint16_t LoadGraphicsTexture(const char* filePath);
PICNIC_GRAPHICS_DLL uint16_t LoadGraphicsFont(const char* filePath);
PICNIC_GRAPHICS_DLL uint16_t LoadGraphicsMaterial(const char* filePath);
PICNIC_GRAPHICS_DLL void GraphicsClear();
