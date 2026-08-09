#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

#include "Pipeline.h"
#include "PixelResources.h"

namespace PixelGraphics
{
	class CameraManager;
	class PASS;
	class PostProcessPass;
	class ForwardRender : public Pipeline
	{
	public:
		ForwardRender(GraphicsCore* core, ResourceManager* resourceManager);
		void PipelineInitialize() override;
		void PipelineRelease() override;
		void Rendering() override;
		void SetRenderingData(RenderingData& renderingData) override;
		bool Resize(int width, int height) override;

	private:
		CameraManager* cameraManager;
		PostProcessPass* postProcessPass;
		std::vector<PASS*> passList;
		RenderTarget mainRenderTarget;
	};
}
