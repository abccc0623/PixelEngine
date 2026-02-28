#pragma once
#include "Core/Module.h"
struct RenderingData;
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
	RenderingData* rendering;
};

