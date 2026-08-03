#include "pch.h"
#include "PostProcessPass.h"
#include "GraphicsCore.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "ModelFactory.h"
#include "CBufferResources.h"

using namespace DirectX::SimpleMath;

PixelGraphics::PostProcessPass::PostProcessPass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager) : PASS(core, resourceManager, cameraManager)
{
	renderingData.renderType = RENDER_TYPE::QUAD;
	renderingData.World[0] = 2.0f;
	renderingData.World[5] = 2.0f;
	renderingData.World[10] = 2.0f;
	renderingData.World[15] = 1.0f;
}

PixelGraphics::PostProcessPass::~PostProcessPass()
{
	source = nullptr;
}

void PixelGraphics::PostProcessPass::SetRenderingData(RenderingData& newRenderingData)
{

}



void PixelGraphics::PostProcessPass::Rendering(RenderTarget& renderTarget)
{
	auto source = &renderTarget;
	if (!source || !source->shaderResourceView)
	{
		return;
	}

	auto shader = shaderFactory->Get(static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_STATIC));
	auto model = modelFactory->Get(static_cast<std::uint16_t>(ResourceDefaultKey::MODEL_QUAD));
	auto texture = textureFactory->Get(static_cast<std::uint16_t>(ResourceDefaultKey::TEXTURE));
	if (!shader || !model || !contextObjectBuffer)
	{
		return;
	}

	ID3D11DeviceContext* context = core->GetDeviceContext();
	BindObjectBuffer(renderingData);

	ID3D11Buffer* vertexBuffer = model->VertexBuffer.Get();
	ID3D11ShaderResourceView* textureView = source->shaderResourceView.Get();
	ID3D11SamplerState* sampler = samplerState.Get();
	UINT stride = model->stride;
	UINT offset = model->Offset;

	context->OMSetDepthStencilState(core->GetDepthDisabledState(), 0);
	context->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(shader->mLayout);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(model->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->VSSetShader(shader->mVertexShader, nullptr, 0);
	context->PSSetShader(shader->mPixelShader, nullptr, 0);
	context->PSSetShaderResources(0, 1, &textureView);
	context->PSSetSamplers(0, 1, &sampler);
	context->DrawIndexed(model->IndexCount, 0, 0);

	//텍스쳐 다시 리셋
	ID3D11ShaderResourceView* nullTexture = nullptr;
	context->PSSetShaderResources(0, 1, &nullTexture);
}

void PixelGraphics::PostProcessPass::BindObjectBuffer(RenderingData& targetRenderingData)
{
	ObjectBuffer objectBuffer = {};
	Matrix world = Matrix::Identity;
	memcpy(&world, targetRenderingData.World, sizeof(targetRenderingData.World));
	objectBuffer.world = world;
	objectBuffer.wvp = world.Transpose();
	objectBuffer.TexMatrix = Matrix::Identity.Transpose();

	ID3D11Buffer* constantBuffer = contextObjectBuffer->buffer.Get();
	core->GetDeviceContext()->UpdateSubresource(constantBuffer, 0, nullptr, &objectBuffer, 0, 0);
	core->GetDeviceContext()->VSSetConstantBuffers(1, 1, &constantBuffer);
}
