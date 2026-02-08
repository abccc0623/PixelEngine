#include "GraphicsEngine.h"
#include "GraphicsCore.h"
#include "CompileShaderManager.h"
#include "Rendering.h"
#include "Factory.h"
#include "RenderringData.h"
GraphicsEngine::GraphicsEngine():
	mShader(nullptr)
{

}

GraphicsEngine::~GraphicsEngine()
{

}

void GraphicsEngine::Initialize(HWND WindowHandle, int Width, int Height)
{
	GraphicsCore::GraphicsInitialize(WindowHandle, Width, Height);
	mShader		= new CompileShaderManager();
	mRender		= new Rendering();
	mFactory	= new Factory();

	mFactory->Initialize();
	mShader->Initialize();
	mRender->Initialize();
}

void GraphicsEngine::Release()
{
	mFactory->Release();
	mShader->Release();
	mRender->Release();
	delete this;
}

void GraphicsEngine::BeginRender(float R, float G, float B, float A)
{
	GraphicsCore::BeginRender(R, G, B, A);
	
	mRender->Update();
}

void GraphicsEngine::EndRender()
{
	GraphicsCore::EndRender();
}

void GraphicsEngine::SetDebugCamera(float* m)
{
	mRender->DebugCameraMatrix = m;
}

void GraphicsEngine::SetRenderCamera(float* m)
{
	mRender->MainCameraMatrix = m;
}

ObjectID GraphicsEngine::LoadTexture(const char* filePath)
{
	return mFactory->CreateTextureResource(filePath);
}

RenderingData* GraphicsEngine::GetRenderingData()
{
	return mFactory->CreateRenderingData();
}

void GraphicsEngine::SetRenderingData(RenderingData* mData)
{
	mFactory->DeleteRenderingData(mData);
}

ObjectID GraphicsEngine::Model_Debug(float* VertexList, int VertexSize, int* IndexList, int indexSize)
{
	return mFactory->CreateDebugModel(VertexList, VertexSize, IndexList, indexSize);
}

ObjectID GraphicsEngine::Model_Debug(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int indexSize)
{
	return mFactory->CreateDebugModel(VertexList,VertexSize,IndexList,indexSize);
}
