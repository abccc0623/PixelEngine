#include "pch.h"
#include "DrawDebugLine.h"
#include "CBufferResources.h"
#include "ModelResources.h"
#include "PixelResources.h"
#include "RenderringData.h"
#include "GraphicsEngine.h"
#include "GraphicsCore.h"
#include <algorithm>
DrawDebugLine::DrawDebugLine()
{
	linebuffer = CreateDynamicLineBuffer(10000);
}

DrawDebugLine::~DrawDebugLine()
{
	if (linebuffer != nullptr)
	{
		linebuffer->Release();
	}
}
void DrawDebugLine::Initialize(GraphicsEngine* engine)
{
	//linebuffer = CreateDynamicLineBuffer(10000);
	//rasterizerState = engine->Get<RasterizerStateResources>("Wired");
	//shader = engine->Get<ShaderResources>("Debug");
	//sampler = CreateSampler();
}

void DrawDebugLine::Set(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 color)
{
	DebugVertex v;
	v.Pos = pos;
	v.Color = color;
	lineList.push_back(v);
}

void DrawDebugLine::Draw()
{
	//if (lineList.empty()) return;
	//
	//auto context = GraphicsCore::GetDeviceContext();
	//
	//D3D11_MAPPED_SUBRESOURCE mappedResource;
	//HRESULT hr = context->Map(linebuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//
	//if (SUCCEEDED(hr))
	//{
	//	// 실제 데이터 복사 (버퍼 크기보다 정점이 많아지지 않게 주의!)
	//	UINT vertexCount = (UINT)(std::min)((size_t)lineList.size(), (size_t)10000);
	//	memcpy(mappedResource.pData, lineList.data(), sizeof(DebugVertex) * vertexCount);
	//
	//	context->Unmap(linebuffer, 0);
	//
	//	// 3. [파이프라인 설정] 선을 그리기 위한 세팅
	//	UINT stride = sizeof(DebugVertex);
	//	UINT offset = 0;
	//	context->IASetVertexBuffers(0, 1, &linebuffer, &stride, &offset);
	//
	//	// 삼각형이 아닌 '선(LINELIST)'으로 해석하도록 설정
	//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//
	//	// 디렉터님이 가져오신 상태값들 적용
	//	context->RSSetState(rasterizerState->rasterizerState); // Wired 상태
	//	context->PSSetSamplers(0, 1, &sampler);
	//
	//	// 쉐이더 바인딩 (이전에 준비하신 디버그 쉐이더)
	//	GraphicsCore::GetDeviceContext()->IASetInputLayout(shader->mLayout);
	//	GraphicsCore::GetDeviceContext()->VSSetShader(shader->mVertexShader, NULL, 0);
	//	GraphicsCore::GetDeviceContext()->PSSetShader(shader->mPixelShader, NULL, 0);
	//
	//	// 4. [최종 출력] 화면에 그리기
	//	context->Draw(vertexCount, 0);
	//}
	//
	//lineList.clear();
}

ID3D11SamplerState* DrawDebugLine::CreateSampler()
{
	/*
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
	//GraphicsCore::GetDevice()->CreateSamplerState(&samplerDesc, &Sampler);
	return Sampler;
	*/
	return nullptr;
}


ID3D11Buffer* DrawDebugLine::CreateDynamicLineBuffer(int MaxVertexCount)
{
	/*
	ID3D11Buffer* VertexBuffer = nullptr;
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(DebugVertex) * MaxVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	GraphicsCore::GetDevice()->CreateBuffer(&vbd, nullptr, &VertexBuffer);
	return VertexBuffer;
	*/
	return nullptr;
}