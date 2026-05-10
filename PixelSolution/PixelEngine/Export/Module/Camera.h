#pragma once
#include "Core/Module.h"
#include "RenderringData.h"
class Camera :public Module
{
public:
	Camera();
	~Camera();
	void Start() override;
	void LastUpdate() override;

	PIXEL_ENGINEDLL void OrthographicProjection();
	PIXEL_ENGINEDLL void PerspectiveProjection();
private:
	RenderingData rendering;
};

