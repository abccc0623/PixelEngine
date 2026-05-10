#pragma once
namespace ECS::Animation2D
{
	struct Animation2DData
	{
		Pixel::Vector3 position{ 0.0f, 0.0f, 0.0f };
		unsigned int bitmask = 1;
		Pixel::Vector4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Pixel::Vector3 scale{ 1.0f, 1.0f, 1.0f };
		unsigned int  unused2;
	};
}

