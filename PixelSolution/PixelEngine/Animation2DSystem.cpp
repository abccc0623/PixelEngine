#include "pch.h"
#include "Animation2D.h"
#include "Animation2DSystem.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "Renderer2D.h"
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
			if (animationlist->play == false) return;
			int playindex = animationlist->selectIndex;
			if (animationlist->animationArray.size() < playindex) return;
			auto& select = animationlist->selectAnimation;

			select.nowFrameTime += dTime * select.animationSpeed;
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
			auto render1 = registry->Get<Renderer2DData>(ID);
			auto render2 = registry->Get<GraphicsData>(ID);
			if (render1 != nullptr)
			{
				render2->renderingData.texture_key = select.textureID;
				render1->TilingX = 1.0f / select.maxFramesX;
				render1->TilingY = 1.0f / select.maxFramesY;
				render1->OffsetX = (select.framesIndex % select.maxFramesX) * render2->renderingData.sprite.TilingX;
				render1->OffsetY = (select.framesIndex / select.maxFramesX) * render2->renderingData.sprite.TilingY;
			}
		});

}

void ECS::Animation2DSystem::EditorUpdate(Registry* registry)
{
	float dTime = GetDeltaTime();

	auto& Chunked = registry->GetChunkedArray<Animation2DData>();
	Chunked.ForEach([registry, dTime](Animation2DData* data, size_t index)
		{
			auto animationlist = registry->Get<Animation2DDList>(data->thisID);
			if (animationlist->play == false) return;
			int playindex = animationlist->selectIndex;
			if (animationlist->animationArray.size() < playindex) return;
			auto& select = animationlist->selectAnimation;

			auto render1 = registry->Get<Renderer2DData>(data->thisID);
			auto render2 = registry->Get<GraphicsData>(data->thisID);
			if (render1 != nullptr)
			{
				render2->renderingData.texture_key = select.textureID;
				render1->TilingX = 1.0f / select.maxFramesX;
				render1->TilingY = 1.0f / select.maxFramesY;
				render1->OffsetX = (select.framesIndex % select.maxFramesX) * render2->renderingData.sprite.TilingX;
				render1->OffsetY = (select.framesIndex / select.maxFramesX) * render2->renderingData.sprite.TilingY;
			}
		});
}

void ECS::Animation2DSystem::Release()
{


}
