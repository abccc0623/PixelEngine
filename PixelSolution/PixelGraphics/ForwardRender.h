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
		~ForwardRender() override;
		bool PipelineInitialize() override;
		void PipelineRelease() override;
		void Rendering() override;
		void SetRenderingData(RenderingData& renderingData) override;
		bool Resize(int width, int height) override;
		bool RequestFrameCapture(int maxWidth);
		int CopyFrameCapture(std::uint8_t* destination, int capacity, int* width, int* height);

	private:
		CameraManager* cameraManager = nullptr;
		PostProcessPass* postProcessPass = nullptr;
		std::vector<PASS*> passList;
		RenderTarget mainRenderTarget;
		bool captureRequested = false;
		int captureMaxWidth = 960;
		int capturedWidth = 0;
		int capturedHeight = 0;
		std::vector<std::uint8_t> capturedPng;
	};
}
