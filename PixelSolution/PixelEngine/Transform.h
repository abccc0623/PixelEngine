#pragma once
#include "PixelMath.h"

namespace ECS::Transform
{
	struct alignas(16) TransformData
	{
		//빠른 계산을 위한 16+16+16 정렬
		Pixel::Vector3 position{ 0.0f, 0.0f, 0.0f };
		unsigned int bitmask = 1;

		Pixel::Vector3 rotation{ 0.0f, 0.0f, 0.0f };
		unsigned int unused1;

		Pixel::Vector3 scale{ 1.0f, 1.0f, 1.0f };
		unsigned int  unused2;
	};
	struct WorldData
	{
		Pixel::Matrix4x4 world;
	};

	void* AddComponent(unsigned int id);
	void* GetComponent(unsigned int id);
	bool HasComponent(unsigned int id);
	std::string BindJit();
}