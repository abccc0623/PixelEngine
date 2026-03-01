#pragma once
#include "Core/Module.h"
struct RenderingData;
using ObjectID = size_t;
class Renderer2D :public Module
{
public:
	Renderer2D();
	~Renderer2D();

	void LastUpdate() override;
	PIXEL_ENGINEDLL void SetTexture(const std::string& name);
private:
	RenderingData* rendering;
	std::string textureName;
	ObjectID textureID = -1;
};

