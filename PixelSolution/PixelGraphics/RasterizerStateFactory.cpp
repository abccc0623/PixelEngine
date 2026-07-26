#include "pch.h"
#include "RasterizerStateFactory.h"

#include "GraphicsCore.h"
#include "ResourceManager.h"

bool PixelGraphics::RasterizerStateFactory::Initialize(GraphicsCore* core)
{
	Clear();
	graphicsCore = core;
	const bool solidCreated = CreateDefaultState(static_cast<std::uint16_t>(ResourceDefaultKey::RASTERIZER_SOLID), "Solid", D3D11_FILL_SOLID, D3D11_CULL_NONE);
	const bool wireCreated = CreateDefaultState(static_cast<std::uint16_t>(ResourceDefaultKey::RASTERIZER_WIRE), "Wire", D3D11_FILL_WIREFRAME, D3D11_CULL_BACK);
	return solidCreated && wireCreated;
}

void PixelGraphics::RasterizerStateFactory::Release()
{
	Clear();
	graphicsCore = nullptr;
}

void PixelGraphics::RasterizerStateFactory::Clear()
{
	rasterizerStates.clear();
}

std::uint16_t PixelGraphics::RasterizerStateFactory::Load(const std::string& name)
{
	for (const auto& [key, state] : rasterizerStates)
	{
		if (state.path == name)
		{
			return key;
		}
	}

	if (name == "Wired" || name == "Wireframe")
	{
		return static_cast<std::uint16_t>(ResourceDefaultKey::RASTERIZER_WIRE);
	}

	return 0;
}

bool PixelGraphics::RasterizerStateFactory::CreateDefaultState(std::uint16_t key, const std::string& name, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode)
{
	ID3D11Device* device = graphicsCore ? graphicsCore->GetDevice() : nullptr;
	if (!device)
	{
		return false;
	}

	D3D11_RASTERIZER_DESC description = {};
	description.FillMode = fillMode;
	description.CullMode = cullMode;
	description.FrontCounterClockwise = false;
	description.DepthClipEnable = true;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	if (FAILED(device->CreateRasterizerState(&description, rasterizerState.GetAddressOf())))
	{
		return false;
	}

	RasterizerStateResources resource = {};
	resource.key = key;
	resource.path = name;
	resource.rasterizerState = std::move(rasterizerState);
	rasterizerStates.emplace(key, std::move(resource));
	return true;
}

RasterizerStateResources* PixelGraphics::RasterizerStateFactory::Get(std::uint16_t key)
{
	const auto found = rasterizerStates.find(key);
	if (found != rasterizerStates.end())
	{
		return &found->second;
	}
	return nullptr;
}
