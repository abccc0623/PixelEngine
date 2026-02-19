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

void PixelGraphicsRendering(float R, float G, float B, float A)
{
	mEngine->BeginRender(R, G, B, A);

	mEngine->EndRender();
}

void PixelGraphicsResize(int Width, int Height)
{
	GraphicsCore::Resize(Width, Height);
}

RenderingData* GetRenderingData()
{
	return mEngine->GetRenderingData();
}

void DeleteRenderingData(RenderingData* mData)
{
	mEngine->DeleteRenderingData(mData);
}

ObjectID LoadGraphicsTexture(const char* filePath)
{
	return mEngine->LoadTexture(filePath);
}

ObjectID CreateDebugModel(Vertex_Debug* Vertex, int VertexSize, int* IndexList, int IndexSize)
{
	return mEngine->Model_Debug(Vertex, VertexSize, IndexList, IndexSize);
}



