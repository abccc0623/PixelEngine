#include "pch.h"
#include "Camera.h"
#include "Module/Transform.h"
#include "Core/GameObject.h"
#include "PixelGraphicsAPI.h"
#include "LuaManager.h"
#include "BindManager.h"
Camera::Camera()
{
	rendering.Type = CAMERA;
}
Camera::~Camera()
{
}

void Camera::Start()
{

}

void Camera::LastUpdate()
{
	PMatrix local = transform->GetLocal();
	for (int i = 0; i < 16; i++)
	{
		rendering.World[i] = local._m[i];
	}
	SetRenderingData(rendering);
}

void Camera::OrthographicProjection()
{
	rendering.camera.Projection = ProjectionType::Orthographic;
}

void Camera::PerspectiveProjection()
{
	rendering.camera.Projection = ProjectionType::Perspective;
}

