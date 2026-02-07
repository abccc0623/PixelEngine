#pragma once
#include <windows.h>

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

	RenderingData* GetRenderingData();
	RenderingData* GetRenderingDataQuid();
	void SetRenderingData(RenderingData* mData);
	int Model_Debug(float* VertexList, int VertexSize, int* IndexList, int indexSize);
	int Model_Debug(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int indexSize);
private:
	CompileShaderManager*	mShader;
	Rendering*				mRender;
	Factory*				mFactory;
};

