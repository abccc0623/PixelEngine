#pragma once
#include "Core/Module.h"
#include "RenderringData.h"
using ObjectID = size_t;
class Renderer2D :public Module
{
public:
	Renderer2D();
	~Renderer2D();

	void LastUpdate() override;
	void SetTexture(const std::string& name);
	void SetMaterial(const std::string& name);
	void SetAnimation(int MaxFramesX, int MaxFramesY, float speed);
private:
	RenderingData rendering;
	std::string textureName;
	ObjectID textureID = -1;
	ObjectID materialID = -1;
	float oneFrameTime = 0.0f;
	float nowFrameTime = 0.0f;
	float animationSpeed = 0.0f;
};

