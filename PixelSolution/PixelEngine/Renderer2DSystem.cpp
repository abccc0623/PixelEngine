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
	auto& renderer2DArray = registry->GetArray<ECS::Renderer2D::Renderer2DData>();
	int size = renderer2DArray.size();
	for (int i = 0; i < size; i++)
	{
		auto id = registry->GetEntityID<ECS::Renderer2D::Renderer2DData>(i);
		auto data = registry->Get<ECS::Transform::WorldData>(id);

		const float* sourcePtr = glm::value_ptr(data->world);
		std::copy(sourcePtr, sourcePtr + 16, renderer2DArray[i].renderingData.World);
		SetRenderingData(renderer2DArray[i].renderingData);
	}
}

void ECS::Renderer2DSystem::Release()
{
}
