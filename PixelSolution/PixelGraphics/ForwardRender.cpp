#include "pch.h"
#include "ForwardRender.h"

#include <cstring>

#include "CBufferResources.h"
#include "GraphicsCore.h"
#include "MaterialFactory.h"
#include "ModelFactory.h"
#include "RasterizerStateFactory.h"
#include "RenderringData.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "TextureFactory.h"

PixelGraphics::ForwardRender::ForwardRender(GraphicsCore* graphicsCore, ResourceManager* resources) : Pipeline(graphicsCore, resources)
{}

bool PixelGraphics::ForwardRender::Initialize()
{
	if (!core || !resourceManager || !textureFactory || !shaderFactory || !rasterizerStateFactory || !modelFactory || !materialFactory)
	{
		return false;
	}

	return CreateConstantBuffer(sizeof(CameraBuffer), cameraBuffer.GetAddressOf()) && CreateConstantBuffer(sizeof(ObjectBuffer), objectBuffer.GetAddressOf()) && CreateSampler();
}

void PixelGraphics::ForwardRender::Release()
{
	samplerState.Reset();
	objectBuffer.Reset();
	cameraBuffer.Reset();
	Pipeline::Release();
}

bool PixelGraphics::ForwardRender::CreateConstantBuffer(UINT byteWidth, ID3D11Buffer** buffer)
{
	if (!core || !core->GetDevice() || !buffer)
	{
		return false;
	}

	D3D11_BUFFER_DESC description = {};
	description.ByteWidth = (byteWidth + 15u) & ~15u;
	description.Usage = D3D11_USAGE_DEFAULT;
	description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	return SUCCEEDED(core->GetDevice()->CreateBuffer(&description, nullptr, buffer));
}

bool PixelGraphics::ForwardRender::CreateSampler()
{
	D3D11_SAMPLER_DESC description = {};
	description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	description.MinLOD = 0.0f;
	description.MaxLOD = D3D11_FLOAT32_MAX;
	return SUCCEEDED(core->GetDevice()->CreateSamplerState(&description, samplerState.GetAddressOf()));
}

void PixelGraphics::ForwardRender::Render(std::vector<RenderingData>& renderingList, const float* backgroundColor)
{
	if (!core || !resourceManager || !backgroundColor)
	{
		return;
	}

	core->BindBackBuffer();
	core->ClearBackBuffer(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	core->ApplyViewport();

	if (renderingList.empty())
	{
		return;
	}

	ID3D11DeviceContext* context = core->GetDeviceContext();
	ID3D11SamplerState* sampler = samplerState.Get();
	context->PSSetSamplers(0, 1, &sampler);
	context->VSSetSamplers(0, 1, &sampler);
	const float blendFactor[4] = {};
	context->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);

	previousModelKey = UINT16_MAX;
	previousMaterialKey = UINT16_MAX;
	previousShaderKey = UINT16_MAX;
	previousTextureKey = UINT16_MAX;
	previousRasterizerKey = UINT16_MAX;

	for (const RenderingData& renderingData : renderingList)
	{
		if (renderingData.Type == RENDER_TYPE::NONE || renderingData.Type == RENDER_TYPE::CAMERA)
		{
			continue;
		}

		DirectModel* model = modelFactory->Get(renderingData.mash_key);
		MaterialResources* material = materialFactory->Get(renderingData.material_key);
		ShaderResources* shader = shaderFactory->Get(renderingData.shader_key);
		TextureResources* texture = textureFactory->Get(renderingData.texture_key);
		if (!model || !material || !shader || !texture)
		{
			continue;
		}

		if (previousModelKey != renderingData.mash_key)
		{
			ID3D11Buffer* vertexBuffer = model->VertexBuffer.Get();
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &model->stride, &model->Offset);
			context->IASetIndexBuffer(model->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			previousModelKey = renderingData.mash_key;
		}

		if (previousShaderKey != renderingData.shader_key)
		{
			context->IASetInputLayout(shader->mLayout);
			context->VSSetShader(shader->mVertexShader, nullptr, 0);
			context->PSSetShader(shader->mPixelShader, nullptr, 0);
			previousShaderKey = renderingData.shader_key;
		}

		if (previousTextureKey != renderingData.texture_key)
		{
			ID3D11ShaderResourceView* textureView = texture->Texture.Get();
			context->PSSetShaderResources(0, 1, &textureView);
			previousTextureKey = renderingData.texture_key;
		}

		if (previousMaterialKey != renderingData.material_key || previousRasterizerKey != material->RasterizerStateKey)
		{
			RasterizerStateResources* rasterizer = rasterizerStateFactory->Get(material->RasterizerStateKey);
			if (rasterizer)
			{
				context->RSSetState(rasterizer->rasterizerState.Get());
				previousRasterizerKey = material->RasterizerStateKey;
			}
			previousMaterialKey = renderingData.material_key;
		}

		context->IASetPrimitiveTopology(renderingData.Type == RENDER_TYPE::BOX2D || renderingData.Type == RENDER_TYPE::LINE ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DirectX::SimpleMath::Matrix world;
		std::memcpy(&world, renderingData.World, sizeof(renderingData.World));
		float tilingX = material->Tiling[0];
		float tilingY = material->Tiling[1];
		float offsetX = material->Offset[0];
		float offsetY = material->Offset[1];
		if (renderingData.Type == RENDER_TYPE::QUAD && !renderingData.sprite.isShared)
		{
			tilingX = renderingData.sprite.TilingX;
			tilingY = renderingData.sprite.TilingY;
			offsetX = renderingData.sprite.OffsetX;
			offsetY = renderingData.sprite.OffsetY;
		}

		ObjectBuffer objectData = {};
		objectData.world = world.Transpose();
		objectData.wvp = (world * GetView() * GetProjection()).Transpose();
		DirectX::SimpleMath::Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(tilingX, tilingY, 1.0f);
		DirectX::SimpleMath::Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(offsetX, offsetY, 0.0f);
		DirectX::SimpleMath::Matrix texMat = texScale * texTrans;
		objectData.TexMatrix = texMat.Transpose();
		context->UpdateSubresource(objectBuffer.Get(), 0, nullptr, &objectData, 0, 0);
		ID3D11Buffer* buffer = objectBuffer.Get();
		context->VSSetConstantBuffers(1, 1, &buffer);
		context->DrawIndexed(model->IndexCount, 0, 0);
	}
}
