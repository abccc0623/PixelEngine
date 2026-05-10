#pragma once
#include "PixelMath.h"
#include "PixelGraphicsAPI.h"
namespace ECS::Renderer2D
{
	struct Renderer2DData
	{
		RenderingData renderingData;
		unsigned long long textureID;
		unsigned long long materialID;
		Pixel::Vector2 Offset{ 1,1 };
		Pixel::Vector2 Tiling{ 0,0 };
	};
	void Add(unsigned int id);
	void SetTexture(unsigned int id,const char* name);
	void SetTextureOffset(unsigned int id,float OffsetX, float OffsetY);
	void AddTextureOffset(unsigned int id,float OffsetX, float OffsetY);
	void SetTextureTiling(unsigned int id,float TilingX, float TilingY);
	void AddTextureTiling(unsigned int id,float TilingX, float TilingY);
}

