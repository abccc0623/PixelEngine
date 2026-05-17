#include "pch.h"
#include "Camera.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
void* ECS::Camera::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<CameraData>(id);

	auto data = registry->Get<CameraData>(id);
	data->renderingData.Type = RENDER_TYPE::CAMERA;
	return data;
}

void* ECS::Camera::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	CameraData* data = registry->Get<CameraData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Camera][GetComponent] Not Find Component");
	}
	return data;
}

bool ECS::Camera::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	CameraData* data = registry->Get<CameraData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

std::string ECS::Camera::BindJit()
{
	std::string jit = R"(
ffi.cdef[[
	typedef struct 
	{ 
		const void* const renderingData;
	} CameraData;
]]
    )";
	return jit;
}
