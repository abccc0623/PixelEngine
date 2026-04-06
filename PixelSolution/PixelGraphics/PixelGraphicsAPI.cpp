#include "pch.h"
#include "PixelGraphicsAPI.h"
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
extern GraphicsEngine* mEngine = nullptr;
bool PixelGraphicsInitialize(HWND hwnd, int Width, int Height)
{
	if(mEngine == nullptr)
	{
		mEngine = new GraphicsEngine();
		mEngine->Initialize(hwnd, Width, Height);
	}
	return true;
}

void PixelGraphicsRelease()
{
	mEngine->Release();
	delete mEngine;
	mEngine = nullptr;
}

void PixelGraphicsRendering()
{
	mEngine->BeginRender();

	mEngine->EndRender();
}

void PixelGraphicsResize(int Width, int Height)
{
	GraphicsCore::Resize(Width, Height);
}

void PixelGraphicsBackgroundColor(int R, int G, int B)
{
	float color[3];
	color[0] = R / 255.0f;
	color[1] = G / 255.0f;
	color[2] = B / 255.0f;
	mEngine->SetBackGroundColor(color);
}

void SetRenderingData(RenderingData& mData)
{
	mEngine->SetRenderingData(mData);
}

ObjectID LoadGraphicsTexture(const char* filePath)
{
	return mEngine->LoadTexture(filePath);
}

ObjectID LoadGraphicsMaterial(const char* filePath)
{
	return mEngine->LoadMaterial(filePath);
}

ObjectID CreateDebugModel(Vertex_Debug* Vertex, int VertexSize, int* IndexList, int IndexSize)
{
	return mEngine->Model_Debug(Vertex, VertexSize, IndexList, IndexSize);
}
void GraphicsClear()
{
	mEngine->Clear();
}



