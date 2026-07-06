#include "pch.h"
#include "CameraSystem.h"
#include "Renderer2D.h"
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
	auto& Chunked = registry->GetChunkedArray<CameraData>();
	Chunked.ForEach([registry](CameraData* data, size_t index)
		{
			auto id = registry->GetEntityID<CameraData>(index);
			auto g = registry->Get<GraphicsData>(index);
			auto world = registry->Get<WorldData>(id);
			if (world != nullptr)
			{
				glm::mat4 viewMatrix = glm::inverse(world->world);
				Pixel::Matrix4x4 camMatrix = viewMatrix;

				const float* sourcePtr = glm::value_ptr(camMatrix);
				std::copy(sourcePtr, sourcePtr + 16, g->renderingData.World);
				SetRenderingData(g->renderingData);
			}
		});
}

void ECS::CameraSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::CameraSystem::Release()
{

}
