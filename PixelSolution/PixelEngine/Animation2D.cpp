#include "pch.h"
#include "Animation2D.h"
#include "Renderer2D.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
#include "Graphics.h"
#include "Scene.h"
#include "SceneManager.h"
#include "EntityObject.h"
#include <cmath>

extern PixelEngine* Engine;
void ApplyAnimationFrame(GraphicsData& graphics, const AnimationFrame& animation)
{
	auto& sprite = graphics.renderingData.sprite;
	sprite.isShared = false;
	graphics.renderingData.texture_key = animation.textureID;
	sprite.TilingX = animation.tilingX;
	sprite.TilingY = animation.tilingY;
	sprite.OffsetX = animation.startOffsetX + (animation.framesIndex % animation.maxFramesX) * animation.tilingX;
	sprite.OffsetY = animation.startOffsetY + (animation.framesIndex / animation.maxFramesX) * animation.tilingY;
}
Animation2DData* Animation2D_Add(unsigned int id)
{
	auto registry = GetRegistry();
	if (!registry->TryAddComponents<Animation2DData, Animation2DDList>(id)) return nullptr;
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
		if (AnimationIndex < 0 || AnimationIndex >= static_cast<int>(data->animationArray.size()))
		{
			PixelLog::Error("[Animation2D][Play] Invalid AnimationIndex");
			return;
		}
		data->play = true;
		if (data->selectIndex != AnimationIndex)
		{
			data->selectIndex = AnimationIndex;
			data->play = true;
			data->selectAnimation = data->animationArray[AnimationIndex];

			auto render = registry->Get<GraphicsData>(id);
			if (render != nullptr)
			{
				render->renderingData.sprite.isShared = false;
				// Apply the new frame before rendering, including callback-triggered transitions.
				const auto& animation = data->selectAnimation;
				if (animation.maxFramesX > 0 && animation.maxFramesY > 0)
				{
					ApplyAnimationFrame(*render, animation);
				}
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
	if (data == nullptr)
	{
		PixelLog::Error("[Animation2D][GetFrameIndex] Not Find");
		return -1;
	}

	if (AnimationIndex < 0 || AnimationIndex >= static_cast<int>(data->animationArray.size()))
	{
		PixelLog::Error("[Animation2D][GetFrameIndex] Invalid AnimationIndex");
		return -1;
	}

	if (data->selectIndex == AnimationIndex)
	{
		return data->selectAnimation.framesIndex;
	}

	return data->animationArray[AnimationIndex].framesIndex;
}

void Animation2D_SetFrameIndex(unsigned int id, int AnimationIndex, int FrameIndex)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Animation2D][SetFrameIndex] Not Find");
		return;
	}

	if (AnimationIndex < 0 || AnimationIndex >= static_cast<int>(data->animationArray.size()))
	{
		PixelLog::Error("[Animation2D][SetFrameIndex] Invalid AnimationIndex");
		return;
	}

	auto& animation = data->animationArray[AnimationIndex];
	const int totalFrames = animation.maxFramesX * animation.maxFramesY;
	if (FrameIndex < 0 || FrameIndex >= totalFrames)
	{
		PixelLog::Error("[Animation2D][SetFrameIndex] Invalid FrameIndex");
		return;
	}

	animation.framesIndex = FrameIndex;
	animation.nowFrameTime = 0.0f;

	if (data->selectIndex == AnimationIndex)
	{
		data->selectAnimation.framesIndex = FrameIndex;
		data->selectAnimation.nowFrameTime = 0.0f;
	}
}

void Animation2D_SetLoop(unsigned int id, int animationIndex, bool loop)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Animation2D][SetLoop] Not Find");
		return;
	}

	if (animationIndex < 0 || animationIndex >= static_cast<int>(data->animationArray.size()))
	{
		PixelLog::Error("[Animation2D][SetLoop] Invalid AnimationIndex");
		return;
	}
	data->animationArray[animationIndex].loop = loop;
	if (data->selectIndex == animationIndex)
	{
		data->selectAnimation.loop = loop;
	}
}

void Animation2D_SetUseUnscaledTime(unsigned int id, bool UseUnscaledTime)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Animation2D][SetUseUnscaledTime] Not Find");
		return;
	}

	data->useUnscaledTime = UseUnscaledTime;
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
		data->selectAnimation.callbackCalled = false;
	}
	else
	{
		PixelLog::Error("[Animation2D][Reset] Not Find");
	}
}


void Animation2D_SetFrameCallback(unsigned int id, int animationIndex, int frameIndex, bool repeat)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Animation2D][SetFrameCallback] Not Find");
		return;
	}

	if (animationIndex < 0 || animationIndex >= static_cast<int>(data->animationArray.size()))
	{
		PixelLog::Error("[Animation2D][SetFrameCallback] Invalid AnimationIndex");
		return;
	}

	auto& animation = data->animationArray[animationIndex];
	const int totalFrames = animation.maxFramesX * animation.maxFramesY;
	if (frameIndex < 0 || frameIndex >= totalFrames)
	{
		PixelLog::Error("[Animation2D][SetFrameCallback] Invalid FrameIndex");
		return;
	}

	animation.callbackFrame = frameIndex;
	animation.callbackRepeat = repeat;
	animation.callbackCalled = false;
	if (data->selectIndex == animationIndex)
	{
		data->selectAnimation.callbackFrame = frameIndex;
		data->selectAnimation.callbackRepeat = repeat;
		data->selectAnimation.callbackCalled = false;
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
		data->animationArray[index].tilingX = 1.0f / MaxFramesX;
		data->animationArray[index].tilingY = 1.0f / MaxFramesY;
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

int Animation2D_CreateRegion(unsigned int id, const char* textureName, int startX, int startY, int frameWidth, int frameHeight, int frameCount, float speed)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DDList>(id);
	if (data == nullptr || Engine == nullptr || textureName == nullptr || textureName[0] == '\0' ||
		startX < 0 || startY < 0 || frameWidth <= 0 || frameHeight <= 0 || frameCount <= 0 || !std::isfinite(speed) || speed <= 0.0f)
	{
		PixelLog::Error("[Animation2D][CreateRegion] Invalid component or arguments");
		return -1;
	}
	const auto textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
	uint32_t width = 0, height = 0;
	if (!GetGraphicsTextureSize(static_cast<uint16_t>(textureID), &width, &height) || width == 0 || height == 0 ||
		static_cast<uint32_t>(startX) >= width || static_cast<uint32_t>(startY) >= height ||
		static_cast<uint32_t>(frameHeight) > height - startY ||
		static_cast<uint32_t>(frameCount) > (width - startX) / static_cast<uint32_t>(frameWidth))
	{
		PixelLog::Error("[Animation2D][CreateRegion] Texture missing or region outside texture");
		return -1;
	}
	const int index = Animation2D_Create(id, textureName, frameCount, 1, speed);
	auto& animation = data->animationArray[index];
	animation.tilingX = static_cast<float>(frameWidth) / width;
	animation.tilingY = static_cast<float>(frameHeight) / height;
	animation.startOffsetX = static_cast<float>(startX) / width;
	animation.startOffsetY = static_cast<float>(startY) / height;
	data->selectAnimation = animation;
	auto graphics = registry->Get<GraphicsData>(id);
	if (graphics != nullptr) ApplyAnimationFrame(*graphics, animation);
	return index;
}
