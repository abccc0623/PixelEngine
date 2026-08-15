#pragma once
#include "PASS.h"
#include "InputLayout.h"

namespace PixelGraphics
{
	class DebugLinePass : public PASS
	{
	public:
		DebugLinePass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager);
		~DebugLinePass() override;

		void SetRenderingData(RenderingData& renderingData) override;
		void Rendering(RenderTarget& renderTarget) override;

	private:
		static constexpr size_t MaxVertexCount = 10000;
		Microsoft::WRL::ComPtr<ID3D11Buffer> lineBuffer;
		std::vector<DebugVertex> lineList;
	};
}
