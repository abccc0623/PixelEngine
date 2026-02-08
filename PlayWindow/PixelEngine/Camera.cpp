#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "PixelGraphicsAPI.h"
Camera::Camera()
{

}
Camera::~Camera()
{

}

void Camera::Start()
{
	transform = targetObject->GetModule<Transform>();
	rendering = GetRenderingData();
	rendering->Type = CAMERA;
	transform->AddPosition(transform->GetLookVector()*-3);
}

void Camera::LastUpdate()
{
	Matrix local = transform->GetLocal();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = local._m[i];
	}
}
