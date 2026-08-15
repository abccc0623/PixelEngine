#include "pch.h"
#include "DebugLinePass.h"

#include "GraphicsCore.h"
#include "RasterizerStateFactory.h"
#include "RenderringData.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"

PixelGraphics::DebugLinePass::DebugLinePass(
	GraphicsCore* core,
	ResourceManager* resourceManager,
	CameraManager* cameraManager)
	: PASS(core, resourceManager, cameraManager)
{
	D3D11_BUFFER_DESC description = {};
	description.Usage = D3D11_USAGE_DYNAMIC;
	description.ByteWidth = static_cast<UINT>(sizeof(DebugVertex) * MaxVertexCount);
	description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	core->GetDevice()->CreateBuffer(&description, nullptr, lineBuffer.GetAddressOf());
	lineList.reserve(MaxVertexCount);
}

PixelGraphics::DebugLinePass::~DebugLinePass() = default;

void PixelGraphics::DebugLinePass::SetRenderingData(RenderingData& renderingData)
{
	if (renderingData.renderType != RENDER_TYPE::LINE || lineList.size() + 2 > MaxVertexCount)
	{
		return;
	}

	const DirectX::XMFLOAT3 color(
		renderingData.line.color[0],
		renderingData.line.color[1],
		renderingData.line.color[2]);

	lineList.push_back({
		DirectX::XMFLOAT3(
			renderingData.line.start[0],
			renderingData.line.start[1],
			renderingData.line.start[2]),
		color });
	lineList.push_back({
		DirectX::XMFLOAT3(
			renderingData.line.end[0],
			renderingData.line.end[1],
			renderingData.line.end[2]),
		color });
}

void PixelGraphics::DebugLinePass::Rendering(RenderTarget& renderTarget)
{
	if (lineList.empty() || !lineBuffer)
	{
		return;
	}

	auto context = core->GetDeviceContext();
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (FAILED(context->Map(lineBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		lineList.clear();
		return;
	}

	memcpy(mapped.pData, lineList.data(), sizeof(DebugVertex) * lineList.size());
	context->Unmap(lineBuffer.Get(), 0);

	UINT stride = sizeof(DebugVertex);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffer = lineBuffer.Get();
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	auto shader = shaderFactory->Get(
		static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_DEBUG));
	if (shader == nullptr)
	{
		lineList.clear();
		return;
	}
	context->IASetInputLayout(shader->mLayout);
	context->VSSetShader(shader->mVertexShader, nullptr, 0);
	context->PSSetShader(shader->mPixelShader, nullptr, 0);

	auto rasterizer = rasterizerStateFactory->Get(
		static_cast<std::uint16_t>(ResourceDefaultKey::RASTERIZER_SOLID));
	if (rasterizer != nullptr)
	{
		context->RSSetState(rasterizer->rasterizerState.Get());
	}

	const float blendFactor[4] = {};
	context->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	context->OMSetDepthStencilState(core->GetDepthEnabledState(), 0);
	context->Draw(static_cast<UINT>(lineList.size()), 0);
	lineList.clear();
}
