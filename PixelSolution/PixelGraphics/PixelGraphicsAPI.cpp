#include "pch.h"
#include "PixelGraphicsAPI.h"
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
extern GraphicsEngine* mEngine = nullptr;
bool PixelGraphicsInitialize(HWND hwnd, int Width, int Height)
{
	if (mEngine == nullptr)
	{
		mEngine = new GraphicsEngine();
		mEngine->Initialize(hwnd, Width, Height);
	}
	return true;
}

void PixelGraphicsRelease()
{
	if (mEngine != nullptr)
	{
		mEngine->Release();
		delete mEngine;
		mEngine = nullptr;
	}
}

void PixelGraphicsRendering()
{
	if (mEngine != nullptr)
	{
		mEngine->GetRenderingManager()->Rendering();
	}
}

void PixelGraphicsResize(int Width, int Height)
{
	if (mEngine != nullptr)
	{
		mEngine->GetGraphicsCore()->Resize(Width, Height);
	}
}

void PixelGraphicsBackgroundColor(int R, int G, int B)
{
	if (mEngine != nullptr)
	{
		float red, green, blue;
		red = R / 255.0f;
		green = G / 255.0f;
		blue = B / 255.0f;
		mEngine->GetRenderingManager()->SetBackgroundColor(red, green, blue);
	}
}

void SetRenderingData(RenderingData& mData)
{
	if (mEngine != nullptr)
	{
		mEngine->GetRenderingManager()->SetRendering(mData);
	}
}

void DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b)
{
	//float start[3] = { x1,y1,z1 };
	//float end[3] = { x2,y2,z2 };
	//float color[3] = { r,g,b };
	//mEngine->DrawLine(start, end, color);
}

void DrawBox2D(float x, float y, float z, float radius, float r, float g, float b)
{
	//float left = x - radius;
	//float right = x + radius;
	//float top = y + radius;
	//float bottom = y - radius;
	//DrawLine(left, top, z, right, top, z, r, g, b);
	//DrawLine(right, top, z, right, bottom, z, r, g, b);
	//DrawLine(right, bottom, z, left, bottom, z, r, g, b);
	//DrawLine(left, bottom, z, left, top, z, r, g, b);
}

void DrawCircle2D(float x, float y, float z, float radius, float r, float g, float b)
{
	//constexpr float PI = 3.14159265358979323846f;
	//float angleStep = (2.0f * PI) / 32;
	//
	//// 시작점 (0도) 계산
	//float prevX = x + radius; // cos(0) = 1
	//float prevY = y;          // sin(0) = 0
	//float prevZ = z;
	//
	//for (int i = 1; i <= 32; ++i)
	//{
	//	float angle = i * angleStep;
	//
	//	// 다음 정점의 좌표 계산
	//	float nextX = x + radius * std::cos(angle);
	//	float nextY = y + radius * std::sin(angle);
	//	float nextZ = z;
	//	// 이전 정점과 현재 정점을 연결하는 선분 렌더링
	//	DrawLine(prevX, prevY, prevZ, nextX, nextY, nextZ, r, g, b);
	//	prevX = nextX;
	//	prevY = nextY;
	//}
}

uint16_t LoadGraphicsTexture(const char* filePath)
{
	if (mEngine != nullptr)
	{
		return mEngine->GetResourceManager()->Load(PixelGraphics::ResourceType::TEXTURE, filePath);
	}
	return 0;
}

uint16_t LoadGraphicsMaterial(const char* filePath)
{
	//return mEngine->LoadMaterial(filePath);
	return 0;
}

ObjectID CreateDebugModel(Vertex_Debug* Vertex, int VertexSize, int* IndexList, int IndexSize)
{
	//return mEngine->Model_Debug(Vertex, VertexSize, IndexList, IndexSize);
	return 0;
}
void GraphicsClear()
{
	//mEngine->Clear();
}



