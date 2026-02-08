#include "Render_Axis.h"
#include "CBufferResources.h"
#include "ModelResources.h"
#include "ShaderResources.h"
#include "RenderringData.h"
Render_Axis::Render_Axis()
{

}

Render_Axis::~Render_Axis()
{

}

void Render_Axis::Binding(RenderingData* mData)
{
	if (mData->Model_ID == -1) { return; }

	ObjectBuffer mbuffer;
	DirectX::SimpleMath::Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	mWorld._11 = mData->World[0];
	mWorld._12 = mData->World[1];
	mWorld._13 = mData->World[2];
	mWorld._14 = mData->World[3];
	mWorld._21 = mData->World[4];
	mWorld._22 = mData->World[5];
	mWorld._23 = mData->World[6];
	mWorld._24 = mData->World[7];
	mWorld._31 = mData->World[8];
	mWorld._32 = mData->World[9];
	mWorld._33 = mData->World[10];
	mWorld._34 = mData->World[11];
	mWorld._41 = mData->World[12];
	mWorld._42 = mData->World[13];
	mWorld._43 = mData->World[14];
	mWorld._44 = 1.0f;
	mbuffer.world = DirectX::XMMatrixTranspose(mWorld);
	mbuffer.TexMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
	GetDeviceContext()->UpdateSubresource(mContextBuffer_List["ObjectBuffer"], 0, nullptr, &mbuffer, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(1, 1, &mContextBuffer_List["ObjectBuffer"]);

	//모델 바인딩
	DirectModel* mAxis = mModelBuffer_List[mData->Model_ID];
	GetDeviceContext()->IASetVertexBuffers(0, 1, &mAxis->VertexBuffer, &mAxis->stride, &mAxis->Offset);
	GetDeviceContext()->IASetIndexBuffer(mAxis->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//레스터라이저바인딩
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	GetDeviceContext()->RSSetState(mRasterizer_List["Wire"]);

	//샘플러바인딩
	ID3D11SamplerState* mSampler = mShaderSampler_List["Basic"];
	GetDeviceContext()->PSSetSamplers(0, 1, &mSampler);
	GetDeviceContext()->VSSetSamplers(0, 1, &mSampler);

	//쉐이더 바인딩
	ShaderResources* mShaderBuffer = mShaderResources_List["Debug"];
	GetDeviceContext()->IASetInputLayout(mShaderBuffer->mLayout);
	GetDeviceContext()->VSSetShader(mShaderBuffer->mVertexShader, NULL, 0);
	GetDeviceContext()->PSSetShader(mShaderBuffer->mPixelShader, NULL, 0);


	GetDeviceContext()->DrawIndexed(mAxis->IndexCount, 0, 0);
}
