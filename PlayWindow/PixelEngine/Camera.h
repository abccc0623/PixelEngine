#pragma once
#include "Module.h"
#include "BindLua.h"
class Transform;
struct RenderingData;
class Camera :public Module , public BindLua<Camera>
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

