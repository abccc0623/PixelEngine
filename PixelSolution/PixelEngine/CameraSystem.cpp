#include "pch.h"
#include "CameraSystem.h"
#include "Registry.h"
#include "Transform.h"
#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>
ECS::CameraSystem::CameraSystem()
{

}

ECS::CameraSystem::~CameraSystem()
{

}

void ECS::CameraSystem::Update(Registry* registry)
{
	auto& cameraArray = registry->GetArray<ECS::Camera::CameraData>();
	int size = cameraArray.size();
	for (int i = 0; i < size; i++)
	{
		auto id = registry->GetEntityID<ECS::Camera::CameraData>(i);
		auto data = registry->Get<ECS::Transform::WorldData>(id);
		if (data != nullptr)
		{
			glm::mat4 viewMatrix = glm::inverse(data->world);
			Pixel::Matrix4x4 camMatrix = viewMatrix;

			const float* sourcePtr = glm::value_ptr(camMatrix);
			std::copy(sourcePtr, sourcePtr + 16, cameraArray[i].renderingData.World);
			SetRenderingData(cameraArray[i].renderingData);
		}
	}
}

void ECS::CameraSystem::Release()
{

}
