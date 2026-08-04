#include "pch.h"
#include "ScenePass.h"
#include "RenderringData.h"
#include "GraphicsCore.h"
#include "ShaderFactory.h"
#include "ResourceManager.h"
PixelGraphics::ScenePass::ScenePass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager) : PASS(core, resourceManager, cameraManager)
{

}

PixelGraphics::ScenePass::~ScenePass()
{

}

void PixelGraphics::ScenePass::SetRenderingData(RenderingData& renderingData)
{
	renderingList.push_back(renderingData);
}

void PixelGraphics::ScenePass::Rendering(RenderTarget& renderTarget)
{
	Sort(renderingList);
	PreviousKeyReset();


	core->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const float blendFactor[4] = {};
	core->GetDeviceContext()->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	core->GetDeviceContext()->OMSetDepthStencilState(core->GetDepthEnabledState(), 0);


	int count = renderingList.size();
	for (int i = 0; i < count; i++)
	{
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

		core->GetDeviceContext()->DrawIndexed(meshIndexCount, 0, 0);
	}
	renderingList.clear();
}
