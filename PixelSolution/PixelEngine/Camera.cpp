#include "pch.h"
#include "Camera.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
void ECS::Camera::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<CameraData>(id);

	auto data = registry->Get<CameraData>(id);
	data->renderingData.Type = RENDER_TYPE::CAMERA;
}
