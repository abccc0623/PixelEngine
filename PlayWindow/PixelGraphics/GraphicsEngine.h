#pragma once
#include <windows.h>

using ObjectID = size_t;
class CompileShaderManager;
class Rendering;
class Factory;
struct RenderingData;
struct Vertex_Debug;
class GraphicsEngine
{
public:
	GraphicsEngine();
	~GraphicsEngine();
public:
	void Initialize(HWND WindowHandle, int Width, int Height);
	void Release();
	void BeginRender(float R, float G, float B, float A);
	void EndRender();
	
	void SetDebugCamera(float* m);
	void SetRenderCamera(float* m);

	ObjectID LoadTexture(const char* filePath);
	RenderingData* GetRenderingData();
	RenderingData* GetRenderingDataQuid();
	void SetRenderingData(RenderingData* mData);
	ObjectID Model_Debug(float* VertexList, int VertexSize, int* IndexList, int indexSize);
	ObjectID Model_Debug(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int indexSize);
private:
	CompileShaderManager*	mShader;
	Rendering*				mRender;
	Factory*				mFactory;
};

