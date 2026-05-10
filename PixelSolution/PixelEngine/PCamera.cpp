#include "pch.h"
#include "PCamera.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
void ECS::Camera::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<CameraData>(id);
}
