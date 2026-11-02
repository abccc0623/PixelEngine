#include "pch.h"
#include "ScenePass.h"
#include "RenderringData.h"
#include "GraphicsCore.h"
#include "ShaderFactory.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include <algorithm>
PixelGraphics::ScenePass::ScenePass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager) : PASS(core, resourceManager, cameraManager)
{

}

PixelGraphics::ScenePass::~ScenePass()
{

}

void PixelGraphics::ScenePass::SetRenderingData(RenderingData& renderingData)
{
	auto data = renderingData;
	if (data.renderType == RENDER_TYPE::QUAD && data.sprite.color[3] < 1.0f) data.transparent = true;
	renderingList.push_back(data);
}

void PixelGraphics::ScenePass::Rendering(RenderTarget& renderTarget)
{
	auto transparentBegin = std::stable_partition(renderingList.begin(), renderingList.end(),
		[](const RenderingData& data) { return !data.transparent; });
	std::vector<RenderingData> opaque(renderingList.begin(), transparentBegin);
	Sort(opaque);
	std::copy(opaque.begin(), opaque.end(), renderingList.begin());
	const auto view = cameraManager->GetView();
	std::stable_sort(transparentBegin, renderingList.end(), [&view](const RenderingData& left, const RenderingData& right)
		{
			using DirectX::SimpleMath::Vector3;
			const auto leftView = Vector3::Transform(Vector3(left.World[12], left.World[13], left.World[14]), view);
			const auto rightView = Vector3::Transform(Vector3(right.World[12], right.World[13], right.World[14]), view);
			return leftView.z > rightView.z;
		});
	PreviousKeyReset();


	core->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const float blendFactor[4] = {};
	core->GetDeviceContext()->OMSetBlendState(nullptr, blendFactor, UINT_MAX);
	core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthEnabledState(), 0);


	int count = renderingList.size();
	for (int i = 0; i < count; i++)
	{
		if (renderingList[i].transparent && (i == 0 || !renderingList[i - 1].transparent))
		{
			core->GetDeviceContext()->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
			core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthReadOnlyState(), 0);
		}
		BindObjectBuffer(renderingList[i]);

		if (previousMeshKey != renderingList[i].mash_key)
		{
			meshIndexCount = BindMesh(renderingList[i]);
			previousMeshKey = renderingList[i].mash_key;
		}

		if (previousTextureKey != renderingList[i].texture_key)
		{
			BindTexture(renderingList[i]);
			previousTextureKey = renderingList[i].texture_key;
		}
		if (previousMaterialKey != renderingList[i].material_key)
		{

			if (previousShaderKey != renderingList[i].shader_key)
			{
				BindShader(renderingList[i]);
				previousShaderKey = renderingList[i].shader_key;
			}


			ID3D11SamplerState* sampler = samplerState.Get();
			core->GetDeviceContext()->PSSetSamplers(0, 1, &sampler);
			core->GetDeviceContext()->VSSetSamplers(0, 1, &sampler);

			BindRasterizer(renderingList[i]);
			previousMaterialKey = renderingList[i].material_key;
		}
		// Depth sorting can change shader order independently of the material key.
		if (previousShaderKey != renderingList[i].shader_key)
		{
			BindShader(renderingList[i]);
			previousShaderKey = renderingList[i].shader_key;
		}

		core->GetDeviceContext()->DrawIndexed(meshIndexCount, 0, 0);
	}
	renderingList.clear();
	core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthEnabledState(), 0);
}
