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
		std::vector<AnimationFrame> animationArray;
		AnimationFrame selectAnimation;
		int selectIndex = 0;
		bool play = false;
	};

	void Add(unsigned int id);
	void Play(unsigned int id, int index);
	void Stop(unsigned int id);
	void Reset(unsigned int id);
	int Create(unsigned int id, std::string textureName, int MaxFramesX, int MaxFramesY, float speed);
}


