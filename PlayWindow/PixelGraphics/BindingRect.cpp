#include "BindingRect.h"
#include "CBufferResources.h"
#include "ModelResources.h"
#include "PixelResources.h"
#include "RenderringData.h"
#include <iostream>
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
BindingRect::BindingRect()
{

}

BindingRect::~BindingRect()
{

}

void BindingRect::Initialize()
{
	targetBuffer = engine->Get<BufferResources>("ObjectBuffer");
	quadModel = engine->Get<DirectModel>("Box2D");
	rasterizerState = engine->Get<RasterizerStateResources>("Wired");
	shader = engine->Get<ShaderResources>("Debug");
	sampler = CreateSampler();
}

void BindingRect::Binding(RenderingData* mData, Handle64 prev)
{
	ObjectBuffer mbuffer;
	DirectX::SimpleMath::Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, mData->World, sizeof(float) * 16);

	mbuffer.world = DirectX::XMMatrixTranspose(mWorld);
	mbuffer.TexMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());

	GraphicsCore::GetDeviceContext()->UpdateSubresource(targetBuffer->buffer, 0, nullptr, &mbuffer, 0, 0);
	GraphicsCore::GetDeviceContext()->VSSetConstantBuffers(1, 1, &(targetBuffer->buffer));


	//모델 바인딩
	if (ModelCheck(mData->master_key, prev) == false)
	{
		GraphicsCore::GetDeviceContext()->IASetVertexBuffers(0, 1, &(quadModel->VertexBuffer), &(quadModel->stride), &(quadModel->Offset));
		GraphicsCore::GetDeviceContext()->IASetIndexBuffer(quadModel->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

	//레스터라이저바인딩
	if (mData->master_key != prev)
	{
		GraphicsCore::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		GraphicsCore::GetDeviceContext()->RSSetState(rasterizerState->rasterizerState);
		GraphicsCore::GetDeviceContext()->PSSetSamplers(0, 1, &sampler);
		GraphicsCore::GetDeviceContext()->VSSetSamplers(0, 1, &sampler);
	}
	//쉐이더 바인딩
	if (ShaderCheck(mData->master_key, prev) == false)
	{
		GraphicsCore::GetDeviceContext()->IASetInputLayout(shader->mLayout);
		GraphicsCore::GetDeviceContext()->VSSetShader(shader->mVertexShader, NULL, 0);
		GraphicsCore::GetDeviceContext()->PSSetShader(shader->mPixelShader, NULL, 0);
	}

	GraphicsCore::GetDeviceContext()->DrawIndexed(quadModel->IndexCount, 0, 0);
}
ID3D11SamplerState* BindingRect::CreateSampler()
{
	ID3D11SamplerState* Sampler = nullptr;
	//기본 샘플러
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