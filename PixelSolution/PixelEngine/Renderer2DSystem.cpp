#include "pch.h"
#include "Renderer2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Renderer2D.h"
#include "Transform.h"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
ECS::Renderer2DSystem::Renderer2DSystem()
{

}

ECS::Renderer2DSystem::~Renderer2DSystem()
{

}

void ECS::Renderer2DSystem::Update(Registry* registry)
{
	auto& Chunked = registry->GetChunkedArray<ECS::Renderer2D::Renderer2DData>();
	Chunked.ForEach([registry](ECS::Renderer2D::Renderer2DData* data, size_t index)
		{
			auto id = registry->GetEntityID<ECS::Renderer2D::Renderer2DData>(index);
			auto world = registry->Get<ECS::Transform::WorldData>(id);
			if (world != nullptr)
			{
				data->renderingData->sprite.OffsetX = data->OffsetX;
				data->renderingData->sprite.OffsetY = data->OffsetY;
				data->renderingData->sprite.TilingX = data->TilingX;
				data->renderingData->sprite.TilingY = data->TilingY;

				const float* sourcePtr = glm::value_ptr(world->world);
				std::copy(sourcePtr, sourcePtr + 16, data->renderingData->World);
				SetRenderingData(*data->renderingData);
			}
		});
}

void ECS::Renderer2DSystem::Release()
{

}
