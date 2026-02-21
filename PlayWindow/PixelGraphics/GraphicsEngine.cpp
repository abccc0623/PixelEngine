#include "GraphicsEngine.h"
#include "GraphicsCore.h"
#include "Rendering.h"
#include "ResourceFactory.h"

#include "PixelResources.h"
#include "ShaderFactory.h"
#include "RasterizerStateFactory.h"
#include "TextureFactory.h"
#include "ModelFactory.h"
#include "RenderingFactory.h"
#include "RenderringData.h"
#include "BufferFactory.h"

GraphicsEngine::GraphicsEngine()
{
	//mRasterizerStateFactory = nullptr;
	//shaderFactory = nullptr;
}

GraphicsEngine::~GraphicsEngine()
{

}

void GraphicsEngine::Initialize(HWND WindowHandle, int Width, int Height)
{
	GraphicsCore::GraphicsInitialize(WindowHandle, Width, Height);

	//∆Â≈‰∏Æ µÓ∑œ
	BindFactory<RasterizerStateResources,RasterizerStateFactory>();
	BindFactory<ShaderResources, ShaderFactory>();
	BindFactory<TextureResources,TextureFactory>();
	BindFactory<DirectModel, ModelFactory>();
	BindFactory<RenderingData,RenderingFactory>();
	BindFactory<BufferResources, BufferFactory>();

	for (auto& k : factoryMap)
	{
		k.second->Initialize();
	}
	mRender = new Rendering();
	mRender->Initialize(this);
}

void GraphicsEngine::Release()
{
	for (auto& k : factoryMap)
	{
		k.second->Release();
		delete k.second;
		k.second = nullptr;
	}
	factoryMap.clear();
	mRender->Release();
	delete mRender;
	GraphicsCore::GraphicsRelease();
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


ObjectID GraphicsEngine::LoadTexture(const char* filePath)
{
	return Set<TextureResources>(filePath);
}

RenderingData* GraphicsEngine::GetRenderingData()
{
	auto render = Get<RenderingData>("");
	mRender->SetRendering(render);
	return render;
}

void GraphicsEngine::DeleteRenderingData(RenderingData* mData)
{
	mRender->DeleteRendering(mData);
	delete mData;
}

void GraphicsEngine::ChangeRenderingData(RenderingData* mData)
{
	mRender->ChangeRendering(mData);
}

ObjectID GraphicsEngine::Model_Debug(float* VertexList, int VertexSize, int* IndexList, int indexSize)
{
	auto f = GetFactory<DirectModel, ModelFactory>();
	return 0;
}

ObjectID GraphicsEngine::Model_Debug(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int indexSize)
{
	auto f = GetFactory<DirectModel, ModelFactory>();
	return 0;
}

