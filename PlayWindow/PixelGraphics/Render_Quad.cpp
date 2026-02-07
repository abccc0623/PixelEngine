#include "Render_Quad.h"
#include "CBufferResources.h"
#include "ModelResources.h"
#include "ShaderResources.h"
#include "RenderringData.h"
Render_Quad::Render_Quad()
{
}

Render_Quad::~Render_Quad()
{
}

void Render_Quad::Binding(RenderingData* mData)
{
	ObjectBuffer mbuffer;
	DirectX::SimpleMath::Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, mData->World, sizeof(float) * 16);

	mbuffer.world = DirectX::XMMatrixTranspose(mWorld);
	mbuffer.TexMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
	GetDeviceContext()->UpdateSubresource(mContextBuffer_List["ObjectBuffer"], 0, nullptr, &mbuffer, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(1, 1, &mContextBuffer_List["ObjectBuffer"]);

	//모델 바인딩
	DirectModel* mAxis = mModelBufferList["Quad"];
	GetDeviceContext()->IASetVertexBuffers(0, 1, &mAxis->VertexBuffer, &mAxis->stride, &mAxis->Offset);
	GetDeviceContext()->IASetIndexBuffer(mAxis->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//레스터라이저바인딩
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GetDeviceContext()->RSSetState(mRasterizer_List["Solid"]);

	//샘플러바인딩
	ID3D11SamplerState* mSampler = mShaderSampler_List["Basic"];
	GetDeviceContext()->PSSetSamplers(0, 1, &mSampler);
	GetDeviceContext()->VSSetSamplers(0, 1, &mSampler);

	//쉐이더 바인딩
	ShaderResources* mShaderBuffer = mShaderResources_List["Debug"];
	GetDeviceContext()->IASetInputLayout(mShaderBuffer->mLayout);
	GetDeviceContext()->VSSetShader(mShaderBuffer->mVertexShader, NULL, 0);
	GetDeviceContext()->PSSetShader(mShaderBuffer->mPixelShader, NULL, 0);

	GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	GetDeviceContext()->DrawIndexed(mAxis->IndexCount, 0, 0);
}
