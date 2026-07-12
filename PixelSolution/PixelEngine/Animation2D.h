#pragma once
#include "PixelEngineDLL.h"

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
	unsigned int thisID;
};

struct Animation2DDList
{
	std::vector<AnimationFrame> animationArray;
	AnimationFrame selectAnimation;
	int selectIndex = 0;
	bool play = false;
};

extern "C" PIXEL_ENGINEDLL Animation2DData* Animation2D_Add(unsigned int id);
extern "C" PIXEL_ENGINEDLL Animation2DData* Animation2D_Get(unsigned int id);
extern "C" PIXEL_ENGINEDLL bool Animation2D_Has(unsigned int id);

extern "C" PIXEL_ENGINEDLL void Animation2D_Play(unsigned int id, int animationIndex);
extern "C" PIXEL_ENGINEDLL void Animation2D_Stop(unsigned int id);
extern "C" PIXEL_ENGINEDLL int Animation2D_Create(unsigned int id, const char* textureName, int MaxFramesX, int MaxFramesY, float speed);





//namespace ECS::Animation2D
//{
//	struct AnimationFrame
//	{
//		unsigned long long textureID;
//		int framesIndex;
//		int maxFramesX;
//		int maxFramesY;
//		float animationSpeed = 1.0f;
//		float oneFrameTime = 0.0f;
//		float nowFrameTime = 0.0f;
//	};
//
//	struct Animation2DData
//	{
//		std::vector<AnimationFrame> animationArray;
//		AnimationFrame selectAnimation;
//		int selectIndex = 0;
//		bool play = false;
//	};
//
//	void Add(unsigned int id);
//	void Play(unsigned int id, int index);
//	void Stop(unsigned int id);
//	void Reset(unsigned int id);
//	int Create(unsigned int id, std::string textureName, int MaxFramesX, int MaxFramesY, float speed);
//}
