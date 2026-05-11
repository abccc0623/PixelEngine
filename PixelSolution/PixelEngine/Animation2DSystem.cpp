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
	auto& animation2DArray = registry->GetArray<ECS::Animation2D::Animation2DData>();
	int size = animation2DArray.size();
	for (int i = 0; i < size; i++)
	{
		if (animation2DArray[i].play == false) continue;
		int playindex = animation2DArray[i].selectIndex;
		if (animation2DArray[i].animationArray.size() < playindex) continue;
		auto& data = animation2DArray[i].selectAnimation;

		data.nowFrameTime += dTime * data.animationSpeed;
		if (data.nowFrameTime >= data.oneFrameTime)
		{
			int totalFrames = data.maxFramesX * data.maxFramesY;
			if (data.framesIndex >= totalFrames - 1)
			{
				data.framesIndex = 0;
			}
			else
			{
				data.framesIndex++;
			}
			data.nowFrameTime -= data.oneFrameTime;
		}
		int ID = registry->GetEntityID<ECS::Animation2D::Animation2DData>(i);
		auto render = registry->Get<ECS::Renderer2D::Renderer2DData>(ID);
		if (render != nullptr)
		{
			render->renderingData.texture_key = data.textureID;
			render->renderingData.sprite.TilingX = 1.0f / data.maxFramesX;
			render->renderingData.sprite.TilingY = 1.0f / data.maxFramesY;
			render->renderingData.sprite.OffsetX = (data.framesIndex % data.maxFramesX) * render->renderingData.sprite.TilingX;
			render->renderingData.sprite.OffsetY = (data.framesIndex / data.maxFramesX) * render->renderingData.sprite.TilingY;
		}
	}
}

void ECS::Animation2DSystem::Release()
{


}
