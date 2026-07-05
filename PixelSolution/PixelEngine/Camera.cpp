#include "pch.h"
#include "Camera.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "Renderer2D.h"
CameraData* Camera_Add(unsigned int id)
{
	auto registry = GetRegistry();
	if (registry->Has<CameraData>(id) == false)
	{
		registry->Add<CameraData>(id);
	}
	if (registry->Has<GraphicsData>(id) == false)
	{
		registry->Add<GraphicsData>(id);
		auto g = registry->Get<GraphicsData>(id);
		g->renderingData.Type = CAMERA;
	}
	return registry->Get<CameraData>(id);
}

CameraData* Camera_Get(unsigned int id)
{
	auto registry = GetRegistry();
	if (registry->Has<CameraData>(id) == false)
	{
		PixelLog::Error("[Camera][GetComponent] Not Find Component");
		return nullptr;
	}

	return registry->Get<CameraData>(id);
}

bool Camera_Has(unsigned int id)
{
	auto registry = GetRegistry();
	return registry->Has<CameraData>(id);
}
