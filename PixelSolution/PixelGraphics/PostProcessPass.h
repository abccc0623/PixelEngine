#pragma once
#include "PASS.h"

namespace PixelGraphics
{
	class PostProcessPass : public PASS
	{
	public:
		PostProcessPass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager);
		~PostProcessPass() override;
		void SetRenderingData(RenderingData& renderingData) override;
		void Rendering(RenderTarget& renderTarget) override;
		void BindObjectBuffer(RenderingData& renderingData) override;
	private:
		RenderTarget* source = nullptr;
		RenderingData renderingData = {};
	};
}
