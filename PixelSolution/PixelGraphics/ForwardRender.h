#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

#include "Pipeline.h"

namespace PixelGraphics
{
	class ForwardRender : public Pipeline
	{
	public:
		ForwardRender(GraphicsCore* core, ResourceManager* resourceManager);
		bool Initialize() override;
		void Render(std::vector<RenderingData>& renderingList, const float* backgroundColor) override;
		void Release() override;

	private:
		bool CreateConstantBuffer(UINT byteWidth, ID3D11Buffer** buffer);
		bool CreateSampler();
		void UpdateCameraBuffer();

		Microsoft::WRL::ComPtr<ID3D11Buffer> objectBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
		std::uint16_t previousModelKey = UINT16_MAX;
		std::uint16_t previousMaterialKey = UINT16_MAX;
		std::uint16_t previousShaderKey = UINT16_MAX;
		std::uint16_t previousTextureKey = UINT16_MAX;
		std::uint16_t previousRasterizerKey = UINT16_MAX;
	};
}
