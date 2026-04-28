#include "pch.h"
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
#include "Rendering.h"
#include "DrawDebugLine.h"
#include "ResourceFactory.h"

#include "PixelResources.h"
#include "ShaderFactory.h"
#include "RasterizerStateFactory.h"
#include "TextureFactory.h"
#include "ModelFactory.h"
#include "RenderringData.h"
#include "BufferFactory.h"
#include "MaterialFactory.h"

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
	BindFactory<BufferResources, BufferFactory>();
	BindFactory<MaterialResources, MaterialFactory>();

	for (auto& k : factoryMap)
	{
		k.second->Initialize();
	}
	mRender = new Rendering();
	mRender->Initialize(this);
	mDebugRender = new DrawDebugLine();
	mDebugRender->Initialize(this);
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
	delete mDebugRender;
	GraphicsCore::GraphicsRelease();
}

void GraphicsEngine::BeginRender()
{
	GraphicsCore::BeginRender(renderColor[0], renderColor[1], renderColor[2], renderColor[3]);
	
	mRender->Update();
	mDebugRender->Draw();
}

void GraphicsEngine::EndRender()
{
	GraphicsCore::EndRender();
}

void GraphicsEngine::Clear()
{
	for (auto& k : factoryMap)
	{
		k.second->Clear();
	}
}

void GraphicsEngine::SetBackGroundColor(float* mColor)
{
	renderColor[0] = mColor[0];
	renderColor[1] = mColor[1];
	renderColor[2] = mColor[2];
}


ObjectID GraphicsEngine::LoadTexture(const char* filePath)
{
	return Set<TextureResources>(filePath);
}

ObjectID GraphicsEngine::LoadMaterial(const char* filePath)
{
	return Set<MaterialResources>(filePath);
}


void GraphicsEngine::SetRenderingData(RenderingData& mData)
{
	mRender->SetRendering(mData);
}

void GraphicsEngine::DrawLine(float start[3], float end[3],float color[3])
{
	DirectX::XMFLOAT3 XMColor;
	XMColor.x = color[0];
	XMColor.y = color[1];
	XMColor.z = color[2];

	DirectX::XMFLOAT3 XMStart;
	XMStart.x = start[0];
	XMStart.y = start[1];
	XMStart.z = start[2];
	DirectX::XMFLOAT3 XMEnd;
	XMEnd.x = end[0];
	XMEnd.y = end[1];
	XMEnd.z = end[2];
	mDebugRender->Set(XMStart,XMColor);
	mDebugRender->Set(XMEnd,XMColor);
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

