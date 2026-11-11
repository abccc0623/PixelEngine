#include "pch.h"
#include "UIPass.h"
#include <algorithm>
#include <cstring>
#include "GraphicsCore.h"
#include "TextureFactory.h"
#include "CameraManager.h"
#include "CBufferResources.h"

using namespace DirectX::SimpleMath;

PixelGraphics::UIPass::UIPass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager)
	: PASS(core, resourceManager, cameraManager), textRenderer(core, resourceManager, cameraManager)
{
	CreateSampler(D3D11_FILTER_MIN_MAG_MIP_POINT);
}

PixelGraphics::UIPass::~UIPass() = default;

void PixelGraphics::UIPass::SetRenderingData(RenderingData& renderingData)
{
	renderingList.push_back(renderingData);
}

void PixelGraphics::UIPass::Rendering(RenderTarget& renderTarget)
{
	Sort(renderingList);
	PreviousKeyReset();
	BindSampler();

	auto* context = core->GetDeviceContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const float blendFactor[4] = {};
	context->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	context->OMSetDepthStencilState(core->GetDepthDisabledState(), 0);

	for (auto& data : renderingList)
	{
		if (data.renderType == RENDER_TYPE::TEXT)
		{
			textRenderer.Draw(data);
			// Text rendering changes shared D3D state, including the sampler.
			PreviousKeyReset();
			BindSampler();
			continue;
		}
		BindObjectBuffer(data);
		BindDrawResources(data);
		context->DrawIndexed(meshIndexCount, 0, 0);
	}
	renderingList.clear();
}

void PixelGraphics::UIPass::Sort(std::vector<RenderingData>& data)
{
	std::stable_sort(data.begin(), data.end(), [](const RenderingData& left, const RenderingData& right)
		{
			// TextData has no Order field; keep its existing order at zero.
			const int leftOrder = left.renderType == RENDER_TYPE::TEXT ? 0 : left.sprite.Order;
			const int rightOrder = right.renderType == RENDER_TYPE::TEXT ? 0 : right.sprite.Order;
			return leftOrder < rightOrder;
		});
}

void PixelGraphics::UIPass::BindObjectBuffer(RenderingData& data)
{
	const auto& sprite = data.sprite;
	float width = sprite.width;
	float height = sprite.height;
	if (data.renderType == RENDER_TYPE::QUAD && (width <= 0.0f || height <= 0.0f))
	{
		auto* texture = textureFactory ? textureFactory->Get(data.texture_key) : nullptr;
		if (texture != nullptr)
		{
			if (width <= 0.0f) width = static_cast<float>(texture->width);
			if (height <= 0.0f) height = static_cast<float>(texture->height);
		}
	}

	Matrix world;
	std::memcpy(&world, data.World, sizeof(data.World));
	const Matrix uiFlip = Matrix::CreateScale(width, -height, 1.0f);
	const Matrix pivotTranslation = Matrix::CreateTranslation(
		-(sprite.pivotX - 0.5f) * width, -(sprite.pivotY - 0.5f) * height, 0.0f);
	const Matrix worldViewProjection = uiFlip * pivotTranslation * world * cameraManager->GetProjUI();

	Matrix textureTransform = Matrix::Identity;
	if (data.renderType == RENDER_TYPE::QUAD && !sprite.isShared)
	{
		const Matrix scale = Matrix::CreateScale(sprite.TilingX, sprite.TilingY, 1.0f);
		const Matrix translation = Matrix::CreateTranslation(sprite.OffsetX, sprite.OffsetY, 0.0f);
		textureTransform = scale * translation;
	}

	ObjectBuffer objectData = {};
	objectData.wvp = worldViewProjection.Transpose();
	objectData.TexMatrix = textureTransform.Transpose();
	objectData.Color = Vector4(sprite.color[0], sprite.color[1], sprite.color[2], sprite.color[3]);
	auto* context = core->GetDeviceContext();
	auto* buffer = contextObjectBuffer->buffer.Get();
	context->UpdateSubresource(buffer, 0, nullptr, &objectData, 0, 0);
	context->VSSetConstantBuffers(1, 1, &buffer);
	context->PSSetConstantBuffers(1, 1, &buffer);
}
