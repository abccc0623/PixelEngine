#include "PixelGraphicsAPI.h"
#include "GraphicsEngine.h"

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
}

void PixelGraphicsRendering(float R, float G, float B, float A)
{
	mEngine->BeginRender(R, G, B, A);

	mEngine->EndRender();
}

void SetRenderCamera(float m[16])
{
	mEngine->SetRenderCamera(m);
}

void SetDebugCamera(float m[16])
{
	mEngine->SetDebugCamera(m);
}

RenderingData* GetRenderingData()
{
	return mEngine->GetRenderingData();
}

void SetRenderingData(RenderingData* mData)
{
	mEngine->SetRenderingData(mData);
}

ObjectID LoadGraphicsTexture(const char* filePath)
{
	return mEngine->LoadTexture(filePath);
}

ObjectID CreateDebugModel(Vertex_Debug* Vertex, int VertexSize, int* IndexList, int IndexSize)
{
	return mEngine->Model_Debug(Vertex, VertexSize, IndexList, IndexSize);
}



