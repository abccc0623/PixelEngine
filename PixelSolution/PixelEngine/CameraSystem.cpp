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
	auto& Chunked = registry->GetChunkedArray<ECS::Camera::CameraData>();
	Chunked.ForEach([registry](ECS::Camera::CameraData* data, size_t index)
		{
			auto id = registry->GetEntityID<ECS::Camera::CameraData>(index);
			auto world = registry->Get<ECS::Transform::WorldData>(id);
			if (world != nullptr)
			{
				glm::mat4 viewMatrix = glm::inverse(world->world);
				Pixel::Matrix4x4 camMatrix = viewMatrix;

				const float* sourcePtr = glm::value_ptr(camMatrix);
				std::copy(sourcePtr, sourcePtr + 16, data->renderingData.World);
				SetRenderingData(data->renderingData);
			}
		});
}

void ECS::CameraSystem::Release()
{

}
