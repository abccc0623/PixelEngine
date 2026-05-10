#pragma once
#include <vector>
#include <string>
namespace ECS::Animation2D
{

	struct AnimationFrame
	{
		unsigned long long textureID;
		int framesIndex;
		int maxFramesX;
		int maxFramesY;
		float animationSpeed = 1.0f;
		float oneFrameTime = 0.0f;
		float nowFrameTime = 0.0f;
	};

	struct Animation2DData
	{
		std::vector<AnimationFrame> animation;
	};

	void Add(unsigned int id);
	void CreateAnimation(unsigned int id, std::string textureName, int MaxFramesX, int MaxFramesY, float speed);
	void PlayAnimation(unsigned int id, int index);
}


