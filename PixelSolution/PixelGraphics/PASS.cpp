#include "pch.h"
#include "PASS.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "TextureFactory.h"
#include "ModelFactory.h"
#include "CBufferResources.h"
#include "RenderringData.h"
#include "MaterialFactory.h"
#include "SimpleMath.h"
#include "PixelResources.h"
#include "CameraManager.h"

using namespace DirectX::SimpleMath;
PixelGraphics::PASS::PASS(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager)
{
	this->core = core;
	this->resourceManager = resourceManager;
	this->cameraManager = cameraManager;
	shaderFactory = resourceManager->GetFactory<ShaderFactory>(PixelGraphics::ResourceType::SHADER);
	textureFactory = resourceManager->GetFactory<TextureFactory>(PixelGraphics::ResourceType::TEXTURE);
	modelFactory = resourceManager->GetFactory<ModelFactory>(PixelGraphics::ResourceType::MODEL);
	materialFactory = resourceManager->GetFactory<MaterialFactory>(ResourceType::MATERIAL);
	rasterizerStateFactory = resourceManager->GetFactory<RasterizerStateFactory>(ResourceType::RASTERIZER_STATE);

	contextObjectBuffer = shaderFactory->GetBuffer("ObjectBuffer");
	CreateSampler();
}
PixelGraphics::PASS::~PASS()
{
	this->core = nullptr;
	this->resourceManager = nullptr;

	shaderFactory = nullptr;
	textureFactory = nullptr;
	contextObjectBuffer = nullptr;
}

void PixelGraphics::PASS::BindObjectBuffer(RenderingData& r)
{
	ObjectBuffer mbuffer = {};
	Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, r.World, sizeof(float) * 16);
	Matrix mWVP = mWorld * cameraManager->GetView() * cameraManager->GetProj();
	Matrix texMat = Matrix::Identity;
	if (r.sprite.isShared == false)
	{
		//만약 이객체에 따로 지정을 했다면 지정한 값으로
		Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(r.sprite.TilingX, r.sprite.TilingY, 1.0f);
		Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(r.sprite.OffsetX, r.sprite.OffsetY, 0.0f);
		texMat = texScale * texTrans;
	}
	else
	{
		//메터리얼에서 값을 가져와야 할것같은데 일단 이렇게 수정
		Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(1, 1, 1.0f);
		Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(0, 0, 0.0f);
		texMat = texScale * texTrans;
	}
	mbuffer.wvp = mWVP.Transpose();
	mbuffer.TexMatrix = texMat.Transpose();
	mbuffer.Color = Vector4(r.sprite.Color[0], r.sprite.Color[1], r.sprite.Color[2], r.sprite.Color[3]);
	auto objectBuffer = contextObjectBuffer->buffer.Get();
	core->GetDeviceContext()->UpdateSubresource(objectBuffer, 0, nullptr, &mbuffer, 0, 0);
	core->GetDeviceContext()->VSSetConstantBuffers(1, 1, &(objectBuffer));
	core->GetDeviceContext()->PSSetConstantBuffers(1, 1, &(objectBuffer));
}

void PixelGraphics::PASS::BindTexture(RenderingData& r)
{
	auto textureResource = textureFactory->Get(r.texture_key);
	ID3D11ShaderResourceView* textureView = textureResource->Texture.Get();
	core->GetDeviceContext()->PSSetShaderResources(0, 1, &(textureView));
}

void PixelGraphics::PASS::BindShader(RenderingData& r)
{
	auto shaderResource = shaderFactory->Get(r.shader_key);
	core->GetDeviceContext()->IASetInputLayout(shaderResource->mLayout);
	core->GetDeviceContext()->VSSetShader(shaderResource->mVertexShader, nullptr, 0);
	core->GetDeviceContext()->PSSetShader(shaderResource->mPixelShader, nullptr, 0);
}

int PixelGraphics::PASS::BindMesh(RenderingData& r)
{
	auto modelResource = modelFactory->Get(r.mash_key);
	ID3D11Buffer* vertexBuffer = modelResource->VertexBuffer.Get();
	UINT stride = modelResource->stride;
	UINT offset = modelResource->Offset;
	core->GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	core->GetDeviceContext()->IASetIndexBuffer(modelResource->IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	return modelResource->IndexCount;
}

void PixelGraphics::PASS::BindRasterizer(RenderingData& r)
{
	auto material = materialFactory->Get(r.material_key);
	RasterizerStateResources* rasterizer = rasterizerStateFactory->Get(material->RasterizerStateKey);
	if (rasterizer)
	{
		core->GetDeviceContext()->RSSetState(rasterizer->rasterizerState.Get());
		previousRasterizerKey = material->RasterizerStateKey;
	}
}

void PixelGraphics::PASS::Sort(std::vector<RenderingData>& data)
{
	std::sort(data.begin(), data.end(), [](const RenderingData& left, const RenderingData& right)
		{
			if (left.mash_key != right.mash_key)
			{
				return left.mash_key < right.mash_key;
			}
			if (left.material_key != right.material_key)
			{
				return left.material_key < right.material_key;
			}
			if (left.shader_key != right.shader_key)
			{
				return left.shader_key < right.shader_key;
			}
			return left.texture_key < right.texture_key;
		});
}

void PixelGraphics::PASS::CreateSampler()
{
	D3D11_SAMPLER_DESC description = {};
	description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	description.ComparisonFunc = D3D11_COMPARISON_NEVER;
	description.MinLOD = 0.0f;
	description.MaxLOD = D3D11_FLOAT32_MAX;
	core->GetDevice()->CreateSamplerState(&description, samplerState.GetAddressOf());
}

void PixelGraphics::PASS::PreviousKeyReset()
{
	previousMeshKey = UINT16_MAX;
	previousMaterialKey = UINT16_MAX;
	previousShaderKey = UINT16_MAX;
	previousTextureKey = UINT16_MAX;
	previousRasterizerKey = UINT16_MAX;
	meshIndexCount = INT64_MAX;
}



