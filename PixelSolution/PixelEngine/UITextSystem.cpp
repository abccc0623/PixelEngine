#include "pch.h"
#include "UITextSystem.h"
#include "UIText.h"
#include "Graphics.h"
#include "Registry.h"
#include <algorithm>
#include <cstring>

ECS::UITextSystem::UITextSystem()
{}

ECS::UITextSystem::~UITextSystem()
{}

void ECS::UITextSystem::Update(Registry* registry)
{
	auto& chunked = registry->GetChunkedArray<UITextData>();
	chunked.ForEach([registry](UITextData* data, size_t index)
		{
			auto graphics = registry->Get<GraphicsData>(data->thisID);
			if (graphics == nullptr)
			{
				return;
			}

			graphics->renderingData.renderType = RENDER_TYPE::TEXT;
			graphics->renderingData.passType = PASS_TYPE::UI;
			auto& target = graphics->renderingData.text;
			const size_t length = (std::min)(data->text.size(), sizeof(target.content) - 1);
			std::memcpy(target.content, data->text.data(), length);
			target.content[length] = '\0';
			target.fontKey = data->fontKey;
			target.scale = data->scale;
			std::copy(std::begin(data->color), std::end(data->color), target.color);
		});
}

void ECS::UITextSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::UITextSystem::Release()
{}
