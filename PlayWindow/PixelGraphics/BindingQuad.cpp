#include "BindingQuad.h"
#include "CBufferResources.h"
#include "ModelResources.h"
#include "PixelResources.h"
#include "RenderringData.h"
#include <iostream>
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
void BindingQuad::Initialize()
{
	targetBuffer = engine->Get<BufferResources>("ObjectBuffer");
	quadModel = engine->Get<DirectModel>("Quad");
	rasterizerState = engine->Get<RasterizerStateResources>("Solid");
	shader = engine->Get<ShaderResources>("Static");
	texture = engine->Get<TextureResources>("Default");
	sampler = CreateSampler();
}

void BindingQuad::Binding(RenderingData* mData, Handle64 prev)
{
	ObjectBuffer mbuffer;
	DirectX::SimpleMath::Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, mData->World, sizeof(float) * 16);
	
	mbuffer.world = DirectX::XMMatrixTranspose(mWorld);
	mbuffer.TexMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());

	GraphicsCore::GetDeviceContext()->UpdateSubresource(targetBuffer->buffer, 0, nullptr, &mbuffer, 0, 0);
	GraphicsCore::GetDeviceContext()->VSSetConstantBuffers(1, 1, &(targetBuffer->buffer));
	
	//텍스쳐 바인딩
	if (TextureCheck(mData->master_key, prev) == false)
	{
		auto k = engine->Get<TextureResources>(mData->texture_key);
		GraphicsCore::GetDeviceContext()->PSSetShaderResources(0, 1, &(k->Texture));
	}
	
	//모델 바인딩
	if (ModelCheck(mData->master_key, prev) == false)
	{
		GraphicsCore::GetDeviceContext()->IASetVertexBuffers(0, 1, &(quadModel->VertexBuffer), &(quadModel->stride), &(quadModel->Offset));
		GraphicsCore::GetDeviceContext()->IASetIndexBuffer(quadModel->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}
	
	//레스터라이저바인딩
	if (mData->master_key != prev)
	{
		GraphicsCore::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	//float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	// 
	//GraphicsCore::GetDeviceContext()->OMSetBlendState(pAlphaBlendState, blendFactor, 0xFFFFFFFF);
	GraphicsCore::GetDeviceContext()->DrawIndexed(quadModel->IndexCount, 0, 0);
	//GraphicsCore::GetDeviceContext()->OMSetDepthStencilState(pDisabledDepthState, 0);
}
ID3D11SamplerState* BindingQuad::CreateSampler()
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