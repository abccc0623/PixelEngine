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
	renderingList.clear();
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

void PixelGraphics::RenderingManager::Update()
{
	std::sort(renderingList.begin(), renderingList.end(), [](const RenderingData& left, const RenderingData& right)
		{
			if (left.mash_key != right.mash_key)
			{
				return left.mash_key < right.mash_key;
			}
			if (left.material_key != right.material_key)
			{
				return left.material_key < right.material_key;
			}
			if (left.shader_key != right.shader_key)
			{
				return left.shader_key < right.shader_key;
			}
			return left.texture_key < right.texture_key;
		});

	if (targetPipeline)
	{
		targetPipeline->Render(renderingList, backgroundColor);
	}
	renderingList.clear();
}

void PixelGraphics::RenderingManager::Rendering()
{
	if (!core || !targetPipeline)
	{
		return;
	}

	Update();
	EndRender();
}

void PixelGraphics::RenderingManager::SetBackgroundColor(float newRed, float newGreen, float newBlue)
{
	backgroundColor[0] = newRed;
	backgroundColor[1] = newGreen;
	backgroundColor[2] = newBlue;
}

void PixelGraphics::RenderingManager::SetRendering(RenderingData& data)
{
	if (data.Type == CAMERA && targetPipeline != nullptr)
	{
		targetPipeline->SetCamera(data);
		return;
	}
	renderingList.push_back(data);
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
		if (!targetPipeline->Initialize())
		{
			delete targetPipeline;
			targetPipeline = nullptr;
			pipelineType = PipelineType::NONE;
			return;
		}
		break;
	default:
		pipelineType = PipelineType::NONE;
		return;
	}

	pipelineType = type;
}
void PixelGraphics::RenderingManager::EndRender()
{
	core->Present();
}
