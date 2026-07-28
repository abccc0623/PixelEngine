#include "pch.h"
#include "Animation2D.h"
#include "Renderer2D.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
#include "Graphics.h"

extern PixelEngine* Engine;
Animation2DData* Animation2D_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<Animation2DData>(id);
	registry->Add<Animation2DDList>(id);
	auto data = registry->Get<Animation2DData>(id);
	data->thisID = id;
	return data;
}

Animation2DData* Animation2D_Get(unsigned int id)
{
	auto registry = GetRegistry();
	return registry->Get<Animation2DData>(id);
}

bool Animation2D_Has(unsigned int id)
{
	auto registry = GetRegistry();
	return registry->Has<Animation2DData>(id);
}

void Animation2D_Play(unsigned int id, int AnimationIndex)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data != nullptr)
	{
		if (data->selectIndex != AnimationIndex)
		{
			data->selectIndex = AnimationIndex;
			data->play = true;
			data->selectAnimation = data->animationArray[AnimationIndex];

			auto render = registry->Get<GraphicsData>(id);
			if (render != nullptr)
			{
				render->renderingData.sprite.isShared = false;
			}
			else
			{
				PixelLog::Error("[Animation2D][Play] Not Find Renderer2D");
			}
		}
	}
	else
	{
		PixelLog::Error("[Animation2D][Play] Not Find");
	}
}

int Animation2D_GetFrameIndex(unsigned int id, int AnimationIndex)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data != nullptr)
	{
		auto animation2d = data->selectAnimation;
		return animation2d.framesIndex;
	}
	else
	{
		PixelLog::Error("[Animation2D][GetFrameIndex] Not Find");
		return -1;
	}
}

void Animation2D_Stop(unsigned int id)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data != nullptr)
	{
		data->play = false;
	}
	else
	{
		PixelLog::Error("[Animation2D][Stop] Not Find");
	}
}

void Animation2D_Reset(unsigned int id)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data != nullptr)
	{
		data->selectAnimation.framesIndex = 0;
		data->selectAnimation.nowFrameTime = 0.0f;
	}
	else
	{
		PixelLog::Error("[Animation2D][Reset] Not Find");
	}
}

int Animation2D_Create(unsigned int id, const char* textureName, int MaxFramesX, int MaxFramesY, float speed)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data != nullptr)
	{
		int index = data->animationArray.size();
		data->animationArray.emplace_back();
		data->animationArray[index].maxFramesX = MaxFramesX;
		data->animationArray[index].maxFramesY = MaxFramesY;
		data->animationArray[index].framesIndex = 0;
		data->animationArray[index].oneFrameTime = 1.0f / MaxFramesX;
		data->animationArray[index].nowFrameTime = 0.0f;
		data->animationArray[index].animationSpeed = speed;
		data->animationArray[index].textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
		data->selectIndex = index;
		data->play = true;
		data->selectAnimation = data->animationArray[index];
		return index;
	}
	return 0;
}
