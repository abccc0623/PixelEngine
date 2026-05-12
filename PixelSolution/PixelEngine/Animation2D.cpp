#include "pch.h"
#include "Animation2D.h"
#include "Renderer2D.h"
#include "Registry.h"
#include "PixelEngineAPI.h"
#include "PixelEngine.h"
#include "ResourceManager.h"

extern PixelEngine* Engine;
void ECS::Animation2D::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Animation2DData>(id);
}

int ECS::Animation2D::Create(unsigned int id, std::string textureName, int MaxFramesX, int MaxFramesY, float speed)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DData>(id);
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
		return index;
	}
	return 0;
}

void ECS::Animation2D::Play(unsigned int id, int index)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DData>(id);
	if (data != nullptr)
	{
		data->selectIndex = index;
		data->play = true;
		data->selectAnimation = data->animationArray[index];

		auto render = registry->Get<ECS::Renderer2D::Renderer2DData>(id);
		if (render != nullptr)
		{
			render->renderingData.sprite.isShared = true;
		}
		else
		{
			PixelLog::Error("[Animation2D] Not Find Renderer2D");
		}
	}
	else
	{
		PixelLog::Error("[Animation2D] Not Find");
	}
}

void ECS::Animation2D::Stop(unsigned int id)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DData>(id);
	if (data != nullptr)
	{
		data->play = false;
	}
	else
	{
		PixelLog::Error("[Animation2D] Not Find");
	}
}

void ECS::Animation2D::Reset(unsigned int id)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Animation2DData>(id);
	if (data != nullptr)
	{
		data->selectAnimation = data->animationArray[data->selectIndex];
	}
	else
	{
		PixelLog::Error("[Animation2D] Not Find");
	}
}
