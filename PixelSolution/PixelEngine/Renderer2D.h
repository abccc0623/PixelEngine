#pragma once
#include "PixelMath.h"
#include "PixelGraphicsAPI.h"
namespace ECS::Renderer2D
{
	struct Renderer2DData
	{
		RenderingData renderingData;
	};
	void Add(unsigned int id);
	void SetTexture(unsigned int id, const char* name);
	void SetTextureOffset(unsigned int id, float OffsetX, float OffsetY);
	void AddTextureOffset(unsigned int id, float OffsetX, float OffsetY);
	void SetTextureTiling(unsigned int id, float TilingX, float TilingY);
	void AddTextureTiling(unsigned int id, float TilingX, float TilingY);
}

