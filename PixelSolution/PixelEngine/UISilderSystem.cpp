#include "pch.h"
#include "UISilderSystem.h"
#include "UISilder.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Transform.h"
#include <glm/gtc/type_ptr.hpp>

void ECS::UISilderSystem::Update(Registry* registry)
{
	auto& sliders = registry->GetChunkedArray<UISilderData>();
	sliders.ForEach([registry](UISilderData* data, size_t index)
		{
			auto world = registry->Get<WorldData>(data->thisID);
			if (world == nullptr) return;

			const float* source = glm::value_ptr(world->world);
			std::copy(source, source + 16, data->background.World);
			SetRenderingData(data->background);

			if (data->fill.sprite.width > 0.0f && data->fill.sprite.height > 0.0f)
			{
				std::copy(source, source + 16, data->fill.World);
				SetRenderingData(data->fill);
			}
		});
}

void ECS::UISilderSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::UISilderSystem::Release()
{
}
