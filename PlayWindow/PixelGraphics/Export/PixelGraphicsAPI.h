#pragma once
#include "PixelGraphicsDLL.h"
#include "RenderringData.h"
#include "VertexData.h"
#include "windows.h"

PICNIC_GRAPHICS_DLL bool PixelGraphicsInitialize(HWND hwnd, int Width, int Height);
PICNIC_GRAPHICS_DLL void PixelGraphicsRelease();
PICNIC_GRAPHICS_DLL void PixelGraphicsRendering(float R = 0.0f, float G = 0.0f, float B = 0.0f, float A = 0.0f);
PICNIC_GRAPHICS_DLL void PixelGraphicsResize(int Width, int Height);

PICNIC_GRAPHICS_DLL RenderingData* GetRenderingData();
PICNIC_GRAPHICS_DLL void DeleteRenderingData(RenderingData* mData);
PICNIC_GRAPHICS_DLL void ChangeRenderingData(RenderingData* mData);

PICNIC_GRAPHICS_DLL ObjectID LoadGraphicsTexture(const char* filePath);
PICNIC_GRAPHICS_DLL ObjectID CreateDebugModel(Vertex_Debug* Vertex,int VertexSize,int* IndexList,int IndexSize);


