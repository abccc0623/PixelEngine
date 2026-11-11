#pragma once
#include "PASS.h"
#include "TextRenderer.h"
namespace PixelGraphics
{
	class UIPass : public PASS
	{
	public:
		UIPass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager);
		~UIPass() override;

		void SetRenderingData(RenderingData& renderingData) override;
		void Rendering(RenderTarget& renderTarget) override;
		void Sort(std::vector<RenderingData>& data) override;
	protected:
		void BindObjectBuffer(RenderingData& renderingData) override;
	private:
		std::vector<RenderingData> renderingList;
		TextRenderer textRenderer;
	};
}

