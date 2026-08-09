#include "pch.h"
#include "RenderingManager.h"
#include <algorithm>

#include "GraphicsCore.h"
#include "ResourceManager.h"
#include "ForwardRender.h"

PixelGraphics::RenderingManager::RenderingManager()
{

}

PixelGraphics::RenderingManager::~RenderingManager()
{
	Release();
}

bool PixelGraphics::RenderingManager::Initialize(GraphicsCore* graphicsCore, ResourceManager* resourceManager)
{
	if (!graphicsCore || !resourceManager)
	{
		return false;
	}
	core = graphicsCore;
	resource = resourceManager;
	ChangePipeline(PipelineType::FORWARD);
	return true;
}

void PixelGraphics::RenderingManager::Release()
{
	if (targetPipeline)
	{
		targetPipeline->Release();
		delete targetPipeline;
		targetPipeline = nullptr;
	}
	pipelineType = PipelineType::NONE;
	core = nullptr;
	resource = nullptr;
}

void PixelGraphics::RenderingManager::Rendering()
{
	if (targetPipeline)
	{
		targetPipeline->Rendering();
	}
}

bool PixelGraphics::RenderingManager::Resize(int width, int height)
{
	return targetPipeline != nullptr && targetPipeline->Resize(width, height);
}

void PixelGraphics::RenderingManager::SetBackgroundColor(float r, float g, float b)
{
	if (targetPipeline != nullptr)
	{
		targetPipeline->SetBackgroundColor(r, g, b);
	}
}

void PixelGraphics::RenderingManager::SetRendering(RenderingData& data)
{
	if (targetPipeline != nullptr)
	{
		targetPipeline->SetRenderingData(data);
	}
}

void PixelGraphics::RenderingManager::ChangePipeline(PipelineType type)
{
	if (pipelineType == type)return;

	if (targetPipeline != nullptr)
	{
		targetPipeline->Release();
		delete targetPipeline;
		targetPipeline = nullptr;
	}

	switch (type)
	{
	case PipelineType::FORWARD:
		targetPipeline = new ForwardRender(core, resource);
		targetPipeline->Initialize();
		break;
	default:
		pipelineType = PipelineType::NONE;
		return;
	}
	pipelineType = type;
}
