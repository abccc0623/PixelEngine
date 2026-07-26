#pragma once
#include <cstdint>
#include "RenderringData.h"
#include "GraphicsCore.h"
#include <map>
#include <vector>
class BindingCamera;
class Render_Axis;
class Render_Quad;
struct RenderingData;
class GraphicsEngine;
class BindingClass;

namespace PixelGraphics
{
	enum class PipelineType : int
	{
		NONE,
		FORWARD,
		DEFERRED,
	};

	class GraphicsCore;
	class ResourceManager;
	class Pipeline;
	class RenderingManager
	{
	public:
		RenderingManager();
		virtual ~RenderingManager();

		bool Initialize(GraphicsCore* core, ResourceManager* resource);
		void Release();
		void Update();

		void Rendering();
		void SetBackgroundColor(float r, float g, float b);
		void SetRendering(RenderingData& data);
		void ChangePipeline(PipelineType type);
	private:
		std::vector<RenderingData> renderingList;
		void EndRender();
		GraphicsCore* core = nullptr;
		ResourceManager* resource = nullptr;
		float backgroundColor[4] = { 0.0f,0.0f, 0.0f, 1.0f };

		Pipeline* targetPipeline = nullptr;
		PipelineType pipelineType = PipelineType::NONE;
	};
}
