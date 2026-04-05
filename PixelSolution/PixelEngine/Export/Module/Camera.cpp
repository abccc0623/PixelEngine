#include "pch.h"
#include "Camera.h"
#include "Module/Transform.h"
#include "Core/GameObject.h"
#include "PixelGraphicsAPI.h"
#include "LuaManager.h"
#include "BindManager.h"
Camera::Camera():
	rendering(nullptr)
{
	rendering = GetRenderingData();
	rendering->Type = CAMERA;
}
Camera::~Camera()
{
	DeleteRenderingData(rendering);
}

void Camera::Start()
{

}

void Camera::LastUpdate()
{
	PMatrix local = transform->GetLocal();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = local._m[i];
	}
}

void Camera::OrthographicProjection()
{
	if (rendering != nullptr)
	{
		rendering->camera.Projection = ProjectionType::Orthographic;
	}
}

void Camera::PerspectiveProjection()
{
	if (rendering != nullptr)
	{
		rendering->camera.Projection = ProjectionType::Perspective;
	}
}

