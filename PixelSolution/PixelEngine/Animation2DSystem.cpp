#include "pch.h"
#include "Animation2D.h"
#include "Animation2DSystem.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "TimeAPI.h"
#include "Renderer2D.h"
#include "Graphics.h"
#include "EntityObject.h"
#include <tuple>
ECS::Animation2DSystem::Animation2DSystem()
{

}

ECS::Animation2DSystem::~Animation2DSystem()
{

}

void ECS::Animation2DSystem::Update(Registry* registry)
{
	const float scaledDeltaTime = GetDeltaTime();
	const float unscaledDeltaTime = Time_GetUnscaledDeltaTime();
	std::vector<std::tuple<unsigned int, int, int>> pendingCallbacks;

	auto& Chunked = registry->GetChunkedArray<Animation2DData>();
	Chunked.ForEach([registry, scaledDeltaTime, unscaledDeltaTime, &pendingCallbacks](Animation2DData* data, size_t index)
		{
			auto animationlist = registry->Get<Animation2DDList>(data->thisID);
			if (animationlist == nullptr) return;
			const float dTime = animationlist->useUnscaledTime ? unscaledDeltaTime : scaledDeltaTime;

			int playindex = animationlist->selectIndex;
			if (playindex < 0 || playindex >= static_cast<int>(animationlist->animationArray.size())) return;

			auto& select = animationlist->selectAnimation;
			if (select.maxFramesX <= 0 || select.maxFramesY <= 0) return;

			if (animationlist->play == true)
			{
				select.nowFrameTime += dTime * select.animationSpeed;
			}
			while (animationlist->play && select.nowFrameTime >= select.oneFrameTime)
			{
				int totalFrames = select.maxFramesX * select.maxFramesY;
				if (select.framesIndex >= totalFrames - 1)
				{
					if (select.loop)
					{
						select.framesIndex = 0;
					}
					else
					{
						select.framesIndex = totalFrames - 1;
						select.nowFrameTime = 0.0f;
						animationlist->play = false;
						break;
					}
				}
				else
				{
					select.framesIndex++;
				}
				select.nowFrameTime -= select.oneFrameTime;
				if (select.framesIndex == select.callbackFrame && (!select.callbackCalled || select.callbackRepeat))
				{
					pendingCallbacks.emplace_back(data->thisID, playindex, select.framesIndex);
					select.callbackCalled = true;
				}
				//애니메이션 루프 여부

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

	for (const auto& callback : pendingCallbacks)
	{
		const unsigned int targetID = std::get<0>(callback);
		auto entity = FindEntity(targetID);
		if (entity != nullptr)
		{
			entity->OnAnimationCallBack(targetID, std::get<1>(callback), std::get<2>(callback));
		}
	}

}

void ECS::Animation2DSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::Animation2DSystem::Release()
{


}
