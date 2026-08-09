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
		void Rendering();
		bool Resize(int width, int height);

		void SetBackgroundColor(float r, float g, float b);
		void SetRendering(RenderingData& data);
		void ChangePipeline(PipelineType type);
	private:
		GraphicsCore* core = nullptr;
		ResourceManager* resource = nullptr;
		Pipeline* targetPipeline = nullptr;
		PipelineType pipelineType = PipelineType::NONE;
	};
}
