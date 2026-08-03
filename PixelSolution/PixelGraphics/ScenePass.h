#pragma once
#include "PASS.h"
namespace PixelGraphics
{
	class ScenePass : public PASS
	{
	public:
		ScenePass(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager);
		virtual ~ScenePass();

		void SetRenderingData(RenderingData& renderingData) override;
		void Rendering(RenderTarget& renderTarget) override;
	private:
		std::vector<RenderingData> renderingList;
	};
}

