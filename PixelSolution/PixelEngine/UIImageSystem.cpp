#include "pch.h"
#include "UIImage.h"
#include "UIImageSystem.h"
#include "PixelGraphicsAPI.h"
#include "Transform.h"
#include "Renderer2D.h"
#include "Registry.h"
#include <algorithm>
#include "Graphics.h"
#include <glm/gtc/type_ptr.hpp>
ECS::UIImageSystem::UIImageSystem()
{

}

ECS::UIImageSystem::~UIImageSystem()
{


}

void ECS::UIImageSystem::Update(ECS::Registry* registry)
{
	auto& Chunked = registry->GetChunkedArray<UIImageData>();
	Chunked.ForEach([registry](UIImageData* data, size_t index)
		{
			auto graphic = registry->Get<GraphicsData>(data->thisID);
			if (graphic != nullptr)
			{
			}
		});
}

void ECS::UIImageSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::UIImageSystem::Release()
{


}
