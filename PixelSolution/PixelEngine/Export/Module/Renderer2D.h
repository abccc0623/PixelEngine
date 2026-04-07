#pragma once
#include "Core/Module.h"
#include "RenderringData.h"
#include <vector>
using ObjectID = size_t;

struct AnimationData
{
	ObjectID textureID;
	int framesIndex;
	int maxFramesX;
	int maxFramesY;
	float animationSpeed = 1.0f;
	float oneFrameTime = 0.0f;
	float nowFrameTime = 0.0f;
};

class Renderer2D :public Module
{
public:
	Renderer2D();
	~Renderer2D();

	void LastUpdate() override;
	void SetTexture(const std::string& name);
	void SetMaterial(const std::string& name);

	int CreateAnimation(std::string textureName,int MaxFramesX, int MaxFramesY, float speed);
	void PlayAnimation(int index);

	void SetTextureOffset(float OffsetX, float OffsetY);
	void AddTextureOffset(float OffsetX, float OffsetY);
	void SetTextureTiling(float TilingX, float TilingY);
	void AddTextureTiling(float TilingX, float TilingY);
private:
	void AnimationClear();

	RenderingData rendering;
	std::string textureName;
	ObjectID textureID = -1;
	ObjectID materialID = -1;

	std::vector<AnimationData> animationList;
	AnimationData nowAnimation;
	int nowAnimationIndex = -1;
};

