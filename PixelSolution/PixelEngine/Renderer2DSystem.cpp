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
	auto& Chunked = registry->GetChunkedArray<Renderer2DData>();
	Chunked.ForEach([registry](Renderer2DData* data, size_t index)
		{
			auto id = registry->GetEntityID<Renderer2DData>(index);
			auto graphic = registry->Get<GraphicsData>(id);
			auto world = registry->Get<WorldData>(id);
			if (world != nullptr)
			{
				graphic->renderingData.sprite.OffsetX = data->OffsetX;
				graphic->renderingData.sprite.OffsetY = data->OffsetY;
				graphic->renderingData.sprite.TilingX = data->TilingX;
				graphic->renderingData.sprite.TilingY = data->TilingY;

				const float* sourcePtr = glm::value_ptr(world->world);
				std::copy(sourcePtr, sourcePtr + 16, graphic->renderingData.World);
				SetRenderingData(graphic->renderingData);
			}
		});
}

void ECS::Renderer2DSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::Renderer2DSystem::Release()
{

}
