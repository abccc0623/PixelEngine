#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

#include "Pipeline.h"
#include "PixelResources.h"

namespace PixelGraphics
{
	class ForwardRender : public Pipeline
	{
	public:
		ForwardRender(GraphicsCore* core, ResourceManager* resourceManager);
		void PipelineInitialize() override;
		void PipelineRelease() override;
		void Rendering() override;
		void SetRenderingData(RenderingData& renderingData) override;
	private:
		void WorldPass();
		void UIPass();

		bool CreateSampler();
		Microsoft::WRL::ComPtr<ID3D11Buffer> objectBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
		std::uint16_t previousModelKey = UINT16_MAX;
		std::uint16_t previousMaterialKey = UINT16_MAX;
		std::uint16_t previousShaderKey = UINT16_MAX;
		std::uint16_t previousTextureKey = UINT16_MAX;
		std::uint16_t previousRasterizerKey = UINT16_MAX;

		RenderTarget renderTarget;

		std::vector<RenderingData> ScenePassList;
		std::vector<RenderingData> UIPassList;
	};
}
