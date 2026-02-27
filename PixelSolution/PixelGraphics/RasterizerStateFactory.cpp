#include "pch.h"
#include "RasterizerStateFactory.h"
#include "GraphicsCore.h"
#include "PixelResources.h"

RasterizerStateFactory::RasterizerStateFactory()
{

}

RasterizerStateFactory::~RasterizerStateFactory()
{

}

void RasterizerStateFactory::Initialize()
{
	rasterizerMap.insert({ "Solid",CreateRasterizerState_Solid()});
	rasterizerMap.insert({ "Wired" ,CreateRasterizerState_Wire()});
}

void RasterizerStateFactory::Release()
{
	for (auto k : rasterizerMap)
	{
		auto value = k.second;
		value->rasterizerState->Release();
		delete value;
		value = nullptr;
		k.second = nullptr;
	}
	rasterizerMap.clear();
}

RasterizerStateResources* RasterizerStateFactory::CreateRasterizerState_Solid()
{
	ID3D11RasterizerState* mSolid = nullptr;
	D3D11_RASTERIZER_DESC solidDesc;
	ZeroMemory(&solidDesc, sizeof(D3D11_RASTERIZER_DESC));
	solidDesc.FillMode = D3D11_FILL_SOLID;
	solidDesc.CullMode = D3D11_CULL_BACK;
	solidDesc.FrontCounterClockwise = false;
	solidDesc.DepthClipEnable = true;
	GraphicsCore::GetDevice()->CreateRasterizerState(&solidDesc, &mSolid);
	auto k = new RasterizerStateResources();
	k->rasterizerState = mSolid;
	return k;
}

RasterizerStateResources* RasterizerStateFactory::CreateRasterizerState_Wire()
{
	ID3D11RasterizerState* mWire = nullptr;
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	GraphicsCore::GetDevice()->CreateRasterizerState(&wireframeDesc, &mWire);
	auto k = new RasterizerStateResources();
	k->rasterizerState = mWire;
	return k;
}

void* RasterizerStateFactory::GetResource(std::string name)
{
	auto value =rasterizerMap.find(name);
	if (value != rasterizerMap.end())
	{
		return value->second;
	}
	else 
	{
		std::cout << "Not Find RasterizerState :" + name << std::endl;
		return nullptr;
	}
}
