#include "pch.h"
#include "Graphics.h"
#include "GraphicsSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Renderer2D.h"
#include "Transform.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
ECS::GraphicsSystem::GraphicsSystem()
{}

ECS::GraphicsSystem::~GraphicsSystem()
{}

void ECS::GraphicsSystem::Update(ECS::Registry* registry)
{
	auto& Chunked = registry->GetChunkedArray<GraphicsData>();
	Chunked.ForEach([registry](GraphicsData* data, size_t index)
		{
			auto id = registry->GetEntityID<GraphicsData>(index);
			auto world = registry->Get<WorldData>(id);
			if (world != nullptr)
			{
				if (data->renderingData.renderType == RENDER_TYPE::CAMERA)
				{
					glm::mat4 viewMatrix = glm::inverse(world->world);
					Pixel::Matrix4x4 camMatrix = viewMatrix;

					const float* sourcePtr = glm::value_ptr(camMatrix);
					std::copy(sourcePtr, sourcePtr + 16, data->renderingData.World);
					SetRenderingData(data->renderingData);
				}
				else
				{
					const float* sourcePtr = glm::value_ptr(world->world);
					std::copy(sourcePtr, sourcePtr + 16, data->renderingData.World);
					SetRenderingData(data->renderingData);
				}
			}
		});
}

void ECS::GraphicsSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::GraphicsSystem::Release()
{}
