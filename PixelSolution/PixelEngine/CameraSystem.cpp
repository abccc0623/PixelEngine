#include "pch.h"
#include "CameraSystem.h"
#include "Renderer2D.h"
#include "Registry.h"
#include "Transform.h"
#include "Graphics.h"
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
			auto g = registry->Get<GraphicsData>(data->thisID);
			auto world = registry->Get<WorldData>(data->thisID);
			if (g != nullptr && world != nullptr)
			{
				g->renderingData.camera.Projection = data->Projection;
				g->renderingData.camera.FovY = data->FovY;
				g->renderingData.camera.NearZ = data->NearZ;
				g->renderingData.camera.FarZ = data->FarZ;
				g->renderingData.camera.ZoomLevel = data->ZoomLevel;
				g->renderingData.camera.ViewportX = data->ViewportX;
				g->renderingData.camera.ViewportY = data->ViewportY;
				g->renderingData.camera.ViewportWidth = data->ViewportWidth;
				g->renderingData.camera.ViewportHeight = data->ViewportHeight;
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
