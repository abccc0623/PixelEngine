#include "pch.h"
#include "UIPass.h"
#include "RenderringData.h"
#include "GraphicsCore.h"
#include "ShaderFactory.h"
#include "TextureFactory.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "CBufferResources.h"
#include "TextRenderer.h"
using namespace DirectX::SimpleMath;
PixelGraphics::UIPass::UIPass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager) : PASS(core, resourceManager, cameraManager)
{
	CreateSampler(D3D11_FILTER_MIN_MAG_MIP_POINT);
	textRenderer = new TextRenderer(core, resourceManager, cameraManager);
}

PixelGraphics::UIPass::~UIPass()
{
	delete textRenderer;
	textRenderer = nullptr;
}

void PixelGraphics::UIPass::SetRenderingData(RenderingData& renderingData)
{
	renderingList.push_back(renderingData);
}

void PixelGraphics::UIPass::Rendering(RenderTarget& renderTarget)
{
	Sort(renderingList);
	PreviousKeyReset();

	ID3D11SamplerState* sampler = samplerState.Get();
	core->GetDeviceContext()->PSSetSamplers(0, 1, &sampler);
	core->GetDeviceContext()->VSSetSamplers(0, 1, &sampler);

	core->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const float blendFactor[4] = {};
	core->GetDeviceContext()->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthDisabledState(), 0);


	int count = renderingList.size();
	for (int i = 0; i < count; i++)
	{
		if (renderingList[i].renderType == RENDER_TYPE::TEXT)
		{
			textRenderer->Draw(renderingList[i]);
			PreviousKeyReset();
			continue;
		}
		//위치 동기화
		BindObjectBuffer(renderingList[i]);

		//매쉬 동기화
		if (previousMeshKey != renderingList[i].mash_key)
		{
			meshIndexCount = BindMesh(renderingList[i]);
			previousMeshKey = renderingList[i].mash_key;
		}

		//메터리얼 바인드
		if (previousMaterialKey != renderingList[i].material_key)
		{
			//내부 쉐이더 바인더
			if (previousTextureKey != renderingList[i].texture_key)
			{
				BindTexture(renderingList[i]);
				previousTextureKey = renderingList[i].texture_key;
			}

			//내부 쉐이더 바인드
			if (previousShaderKey != renderingList[i].shader_key)
			{
				BindShader(renderingList[i]);
				previousShaderKey = renderingList[i].shader_key;
			}

			//Rasterizer 동기화
			BindRasterizer(renderingList[i]);
		}

		core->GetDeviceContext()->DrawIndexed(meshIndexCount, 0, 0);
	}
	renderingList.clear();
}

void PixelGraphics::UIPass::Sort(std::vector<RenderingData>& data)
{
	std::stable_sort(data.begin(), data.end(), [](const RenderingData& left, const RenderingData& right)
		{
			const int leftOrder = left.renderType == RENDER_TYPE::TEXT ? 0 : left.sprite.Order;
			const int rightOrder = right.renderType == RENDER_TYPE::TEXT ? 0 : right.sprite.Order;
			return leftOrder < rightOrder;
		});
}

void PixelGraphics::UIPass::BindObjectBuffer(RenderingData& r)
{
	ObjectBuffer mbuffer = {};
	Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, r.World, sizeof(float) * 16);

	float width = r.sprite.width;
	float height = r.sprite.height;
	if (r.renderType == RENDER_TYPE::QUAD && (width <= 0.0f || height <= 0.0f))
	{
		auto texture = textureFactory ? textureFactory->Get(r.texture_key) : nullptr;
		if (texture != nullptr)
		{
			if (width <= 0.0f) width = static_cast<float>(texture->width);
			if (height <= 0.0f) height = static_cast<float>(texture->height);
		}
	}

	const float pivotOffsetX = (r.sprite.pivotX - 0.5f) * width;
	const float pivotOffsetY = (r.sprite.pivotY - 0.5f) * height;
	Matrix uiFlip = Matrix::CreateScale(width, -height, 1.0f);
	Matrix pivotTranslation = Matrix::CreateTranslation(-pivotOffsetX, -pivotOffsetY, 0.0f);
	Matrix mWVP = uiFlip * pivotTranslation * mWorld * cameraManager->GetProjUI();
	Matrix texMat = Matrix::Identity;

	if (r.renderType == RENDER_TYPE::QUAD && r.sprite.isShared == false)
	{
		Matrix texScale = DirectX::SimpleMath::Matrix::CreateScale(r.sprite.TilingX, r.sprite.TilingY, 1.0f);
		Matrix texTrans = DirectX::SimpleMath::Matrix::CreateTranslation(r.sprite.OffsetX, r.sprite.OffsetY, 0.0f);
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
	mbuffer.Color = Vector4(r.sprite.color[0], r.sprite.color[1], r.sprite.color[2], r.sprite.color[3]);
	auto targetBuffer = contextObjectBuffer->buffer.Get();
	core->GetDeviceContext()->UpdateSubresource(targetBuffer, 0, nullptr, &mbuffer, 0, 0);
	core->GetDeviceContext()->VSSetConstantBuffers(1, 1, &(targetBuffer));
	core->GetDeviceContext()->PSSetConstantBuffers(1, 1, &(targetBuffer));
}
