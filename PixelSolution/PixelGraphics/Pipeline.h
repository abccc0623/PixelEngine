#pragma once
struct RenderingData;
namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceManager;
	class Pipeline
	{
	public:
		Pipeline(GraphicsCore* core, ResourceManager* resourceManager);
		virtual ~Pipeline() = default;
		bool Initialize();
		void Release();
		virtual void PipelineInitialize() = 0;
		virtual void PipelineRelease() = 0;
		virtual void Rendering() = 0;
		virtual void SetRenderingData(RenderingData& renderingData) = 0;

		void SetBackgroundColor(float r, float g, float b);
	protected:
		GraphicsCore* core = nullptr;
		ResourceManager* resourceManager = nullptr;
	protected:
		float backgroundColor[4] = { 1.0f,1.0f, 1.0f, 1.0f };

	};
}

