#pragma once
#include "PixelMath.h"

namespace ECS::Transform
{
	struct alignas(16) TransformData
	{
		//빠른 계산을 위한 16+16+16 정렬
		Pixel::Vector3 position{ 0.0f, 0.0f, 0.0f };
		unsigned int bitmask;
		
		Pixel::Vector4 rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
		
		Pixel::Vector3 scale{ 1.0f, 1.0f, 1.0f};
		unsigned int  unused2;
	};
	struct WorldData
	{
		Pixel::Matrix4x4 world;
	};

	void Add(unsigned int id);
	void SetPosition(unsigned int id, float x, float y, float z);
	void SetRotation(unsigned int id, float x, float y, float z);
	void SetScale(unsigned int id, float x, float y, float z);
}