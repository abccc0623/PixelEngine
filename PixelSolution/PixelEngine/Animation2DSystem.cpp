#include "pch.h"
#include "Animation2D.h"
#include "Animation2DSystem.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "Renderer2D.h"
#include "Graphics.h"
ECS::Animation2DSystem::Animation2DSystem()
{

}

ECS::Animation2DSystem::~Animation2DSystem()
{

}

void ECS::Animation2DSystem::Update(Registry* registry)
{
	float dTime = GetDeltaTime();

	auto& Chunked = registry->GetChunkedArray<Animation2DData>();
	Chunked.ForEach([registry, dTime](Animation2DData* data, size_t index)
		{
			auto animationlist = registry->Get<Animation2DDList>(data->thisID);
			int playindex = animationlist->selectIndex;
			if (animationlist->animationArray.size() < playindex) return;
			auto& select = animationlist->selectAnimation;

			if (animationlist->play == true)
			{
				select.nowFrameTime += dTime * select.animationSpeed;
			}
			if (select.nowFrameTime >= select.oneFrameTime)
			{
				int totalFrames = select.maxFramesX * select.maxFramesY;
				if (select.framesIndex >= totalFrames - 1)
				{
					select.framesIndex = 0;
				}
				else
				{
					select.framesIndex++;
				}
				select.nowFrameTime -= select.oneFrameTime;
			}
			int ID = registry->GetEntityID<Animation2DData>(index);
			auto graphics = registry->Get<GraphicsData>(ID);
			if (graphics != nullptr)
			{
				graphics->renderingData.texture_key = select.textureID;
				graphics->renderingData.sprite.TilingX = 1.0f / select.maxFramesX;
				graphics->renderingData.sprite.TilingY = 1.0f / select.maxFramesY;
				graphics->renderingData.sprite.OffsetX = (select.framesIndex % select.maxFramesX) * graphics->renderingData.sprite.TilingX;
				graphics->renderingData.sprite.OffsetY = (select.framesIndex / select.maxFramesX) * graphics->renderingData.sprite.TilingY;
			}
		});

}

void ECS::Animation2DSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::Animation2DSystem::Release()
{


}
