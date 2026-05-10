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
		auto worldData = registry->Get<ECS::Transform::WorldData>(id);

		glm::mat4 viewMatrix = glm::inverse(worldData->world);
		worldData->world = viewMatrix;

		cameraArray[i].renderingData.Type = CAMERA;
		const float* sourcePtr = glm::value_ptr(worldData->world);
		std::copy(sourcePtr, sourcePtr + 16, cameraArray[i].renderingData.World);
		SetRenderingData(cameraArray[i].renderingData);
	}
}

void ECS::CameraSystem::Release()
{

}
