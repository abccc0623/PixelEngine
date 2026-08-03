#include "pch.h"
#include "Pipeline.h"
#include "GraphicsCore.h"
#include "ResourceManager.h"

PixelGraphics::Pipeline::Pipeline(GraphicsCore* graphicsCore, ResourceManager* resources)
{
	core = graphicsCore;
	resourceManager = resources;
}

bool PixelGraphics::Pipeline::Initialize()
{
	PipelineInitialize();
	return true;
}

void PixelGraphics::Pipeline::Release()
{


}

void PixelGraphics::Pipeline::SetBackgroundColor(float r, float g, float b)
{
	backgroundColor[0] = r;
	backgroundColor[1] = g;
	backgroundColor[2] = b;
	backgroundColor[3] = 1.0f;
}
