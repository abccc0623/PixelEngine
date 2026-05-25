#pragma once
#include "PixelMath.h"
#include "PixelGraphicsAPI.h"
namespace ECS::Renderer2D
{
	struct Renderer2DData
	{
		float TilingX = 1.0f;
		float TilingY = 1.0f;
		float OffsetX = 1.0f;
		float OffsetY = 1.0f;
		RenderingData renderingData;
	};

	void* AddComponent(unsigned int id);
	void* GetComponent(unsigned int id);
	bool HasComponent(unsigned int id);
	void SetTexture(unsigned int id, const char* name);
	std::string BindJit();
}

