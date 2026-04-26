#include "pch.h"
#include "BindingLine.h"
#include "CBufferResources.h"
#include "ModelResources.h"
#include "PixelResources.h"
#include "RenderringData.h"
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
void BindingLine::Initialize()
{
	targetBuffer = engine->Get<BufferResources>("ObjectBuffer");
	quadModel = engine->Get<DirectModel>("Quad");
	rasterizerState = engine->Get<RasterizerStateResources>("Wired");
	shader = engine->Get<ShaderResources>("Static");
	sampler = CreateSampler();
}

void BindingLine::Binding(RenderingData* mData, Handle64 prev)
{
	ObjectBuffer mbuffer = {};
	DirectX::SimpleMath::Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, mData->World, sizeof(float) * 16);

}

ID3D11SamplerState* BindingLine::CreateSampler()
{
	ID3D11SamplerState* Sampler = nullptr;
	//±âº» »ùÇÃ·¯
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	GraphicsCore::GetDevice()->CreateSamplerState(&samplerDesc, &Sampler);
	return Sampler;
}
