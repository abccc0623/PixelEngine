#pragma once
#include "Module.h"
class Transform;
struct RenderingData;
class Camera :public Module
{
public:
	Camera();
	~Camera();

	void Start() override;
	void LastUpdate() override;
	void OrthographicProjection();
	void PerspectiveProjection();

	static void RegisterLua();
private:
	Transform* transform;
	RenderingData* rendering;
};

