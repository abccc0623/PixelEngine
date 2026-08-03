#pragma once
#include "PASS.h"
namespace PixelGraphics
{
	class UIPass : public PASS
	{
	public:
		UIPass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager);
		virtual ~UIPass();

		void SetRenderingData(RenderingData& renderingData) override;
		void Rendering(RenderTarget& renderTarget) override;
		void Sort(std::vector<RenderingData>& data) override;
	protected:
		void BindObjectBuffer(RenderingData& renderingData) override;
	private:
		std::vector<RenderingData> renderingList;
	};
}

