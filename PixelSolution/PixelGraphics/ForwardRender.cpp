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
#include"ModelResources.h"

using namespace DirectX::SimpleMath;
PixelGraphics::ForwardRender::ForwardRender(GraphicsCore* graphicsCore, ResourceManager* resources) : Pipeline(graphicsCore, resources)
{

}
void PixelGraphics::ForwardRender::PipelineInitialize()
{
	BufferResources* cameraBufferResource = shaderFactory->GetBuffer("CameraBuffer");
	BufferResources* objectBufferResource = shaderFactory->GetBuffer("ObjectBuffer");
	cameraBuffer = cameraBufferResource->buffer;
	objectBuffer = objectBufferResource->buffer;
}

void PixelGraphics::ForwardRender::PipelineRelease()
{
	samplerState.Reset();
	objectBuffer.Reset();
	cameraBuffer.Reset();
	ScenePassList.clear();
	UIPassList.clear();
}



void PixelGraphics::ForwardRender::WorldPass()
{
	Sort(ScenePassList);

	previousModelKey = UINT16_MAX;
	previousMaterialKey = UINT16_MAX;
	previousShaderKey = UINT16_MAX;
	previousTextureKey = UINT16_MAX;
	previousRasterizerKey = UINT16_MAX;

	core->GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);
	core->GetDeviceContext()->VSSetSamplers(0, 1, &samplerState);
	const float blendFactor[4] = {};
	core->GetDeviceContext()->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthEnabledState(), 0);

	core->BindBackBuffer();
	core->ClearBackBuffer(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	core->ApplyViewport();
	core->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& K : ScenePassList)
	{
		//오브젝트 정보 세팅
		ObjectBuffer mbuffer = {};
		Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
		memcpy(&mWorld, K.World, sizeof(float) * 16);
		Matrix mWVP = mWorld * GetCameraView() * GetCameraProjection();
		Matrix texMat = Matrix::Identity;
		if (K.renderType == RENDER_TYPE::QUAD && K.sprite.isShared == false)
		{
			Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(K.sprite.TilingX, K.sprite.TilingY, 1.0f);
			Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(K.sprite.OffsetX, K.sprite.OffsetY, 0.0f);
			texMat = texScale * texTrans;
		}
		else
		{
			Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(1, 1, 1.0f);
			Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(0, 0, 0.0f);
			texMat = texScale * texTrans;
		}
		mbuffer.wvp = mWVP.Transpose();
		mbuffer.TexMatrix = texMat.Transpose();
		auto targetBuffer = objectBuffer.Get();
		core->GetDeviceContext()->UpdateSubresource(targetBuffer, 0, nullptr, &mbuffer, 0, 0);
		core->GetDeviceContext()->VSSetConstantBuffers(1, 1, &(targetBuffer));

		//텍스쳐 바인딩
		if (previousTextureKey != K.texture_key)
		{
			auto textureResource = textureFactory->Get(K.texture_key);
			ID3D11ShaderResourceView* textureView = textureResource->Texture.Get();
			core->GetDeviceContext()->PSSetShaderResources(0, 1, &(textureView));
			previousTextureKey = K.texture_key;
		}

		//쉐이더 바인딩
		if (previousShaderKey != K.shader_key)
		{
			auto shaderResource = shaderFactory->Get(K.shader_key);
			core->GetDeviceContext()->IASetInputLayout(shaderResource->mLayout);
			core->GetDeviceContext()->VSSetShader(shaderResource->mVertexShader, nullptr, 0);
			core->GetDeviceContext()->PSSetShader(shaderResource->mPixelShader, nullptr, 0);
			previousShaderKey = K.shader_key;
		}

		if (previousMaterialKey != K.material_key || previousRasterizerKey)
		{
			auto materialResource = materialFactory->Get(K.material_key);
			RasterizerStateResources* rasterizer = rasterizerStateFactory->Get(materialResource->RasterizerStateKey);
			if (rasterizer)
			{
				core->GetDeviceContext()->RSSetState(rasterizer->rasterizerState.Get());
				previousRasterizerKey = materialResource->RasterizerStateKey;
			}
			previousMaterialKey = K.material_key;
		}

		//모델 바인딩
		DirectModel* targetModel = modelFactory->Get(K.mash_key);
		if (targetModel == nullptr)
		{
			continue;
		}
		if (previousModelKey != K.mash_key)
		{
			ID3D11Buffer* vertexBuffer = targetModel->VertexBuffer.Get();
			UINT stride = targetModel->stride;
			UINT offset = targetModel->Offset;
			core->GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			core->GetDeviceContext()->IASetIndexBuffer(targetModel->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			previousModelKey = K.mash_key;
		}
		core->GetDeviceContext()->DrawIndexed(targetModel->IndexCount, 0, 0);
	}
}

void PixelGraphics::ForwardRender::UIPass()
{
	SortUI(UIPassList);
	const Matrix uiProjection = GetUIProjection();
	core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthDisabledState(), 0);

	previousModelKey = UINT16_MAX;
	previousMaterialKey = UINT16_MAX;
	previousShaderKey = UINT16_MAX;
	previousTextureKey = UINT16_MAX;
	previousRasterizerKey = UINT16_MAX;

	//core->GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);
	//core->GetDeviceContext()->VSSetSamplers(0, 1, &samplerState);
	//const float blendFactor[4] = {};
	//core->GetDeviceContext()->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	//core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthEnabledState(), 0);
	//
	//core->BindBackBuffer();
	//core->ClearBackBuffer(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	//core->ApplyViewport();
	//core->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& K : UIPassList)
	{
		//오브젝트 정보 세팅
		ObjectBuffer mbuffer = {};
		Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
		memcpy(&mWorld, K.World, sizeof(float) * 16);
		Matrix uiFlip = Matrix::CreateScale(1.0f, -1.0f, 1.0f);
		Matrix mWVP = uiFlip * mWorld * uiProjection;
		Matrix texMat = Matrix::Identity;
		if (K.renderType == RENDER_TYPE::QUAD && K.sprite.isShared == false)
		{
			Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(K.sprite.TilingX, K.sprite.TilingY, 1.0f);
			Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(K.sprite.OffsetX, K.sprite.OffsetY, 0.0f);
			texMat = texScale * texTrans;
		}
		else
		{
			Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(1, 1, 1.0f);
			Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(0, 0, 0.0f);
			texMat = texScale * texTrans;
		}
		mbuffer.wvp = mWVP.Transpose();
		mbuffer.TexMatrix = texMat.Transpose();
		auto targetBuffer = objectBuffer.Get();
		core->GetDeviceContext()->UpdateSubresource(targetBuffer, 0, nullptr, &mbuffer, 0, 0);
		core->GetDeviceContext()->VSSetConstantBuffers(1, 1, &(targetBuffer));

		//텍스쳐 바인딩
		if (previousTextureKey != K.texture_key)
		{
			auto textureResource = textureFactory->Get(K.texture_key);
			ID3D11ShaderResourceView* textureView = textureResource->Texture.Get();
			core->GetDeviceContext()->PSSetShaderResources(0, 1, &(textureView));
			previousTextureKey = K.texture_key;
		}

		//쉐이더 바인딩
		if (previousShaderKey != K.shader_key)
		{
			auto shaderResource = shaderFactory->Get(K.shader_key);
			core->GetDeviceContext()->IASetInputLayout(shaderResource->mLayout);
			core->GetDeviceContext()->VSSetShader(shaderResource->mVertexShader, nullptr, 0);
			core->GetDeviceContext()->PSSetShader(shaderResource->mPixelShader, nullptr, 0);
			previousShaderKey = K.shader_key;
		}

		if (previousMaterialKey != K.material_key || previousRasterizerKey)
		{
			auto materialResource = materialFactory->Get(K.material_key);
			RasterizerStateResources* rasterizer = rasterizerStateFactory->Get(materialResource->RasterizerStateKey);
			if (rasterizer)
			{
				core->GetDeviceContext()->RSSetState(rasterizer->rasterizerState.Get());
				previousRasterizerKey = materialResource->RasterizerStateKey;
			}
			previousMaterialKey = K.material_key;
		}

		//모델 바인딩
		DirectModel* targetModel = modelFactory->Get(K.mash_key);
		if (targetModel == nullptr)
		{
			continue;
		}
		if (previousModelKey != K.mash_key)
		{
			ID3D11Buffer* vertexBuffer = targetModel->VertexBuffer.Get();
			UINT stride = targetModel->stride;
			UINT offset = targetModel->Offset;
			core->GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			core->GetDeviceContext()->IASetIndexBuffer(targetModel->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			previousModelKey = K.mash_key;
		}
		core->GetDeviceContext()->DrawIndexed(targetModel->IndexCount, 0, 0);
	}
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

void PixelGraphics::ForwardRender::SetRenderingData(RenderingData& renderingData)
{
	if (renderingData.renderType == RENDER_TYPE::CAMERA)
	{
		CameraSetting(renderingData);
	}
	else if (renderingData.passType == PASS_TYPE::SCENE)
	{
		ScenePassList.push_back(renderingData);
	}
	else if (renderingData.passType == PASS_TYPE::UI)
	{
		UIPassList.push_back(renderingData);
	}
	else
	{

	}
}

void PixelGraphics::ForwardRender::Rendering()
{
	WorldPass();
	UIPass();

	core->Present();
	ScenePassList.clear();
	UIPassList.clear();
}
