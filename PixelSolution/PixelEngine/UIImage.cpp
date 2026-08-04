#include "pch.h"
#include "UIImage.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "Renderer2D.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
#include "Graphics.h"
extern PixelEngine* Engine;
UIImageData* UIImage_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<UIImageData>(id);
	if (registry->Has<GraphicsData>(id) == false)
	{
		registry->Add<GraphicsData>(id);
	}
	auto data = registry->Get<UIImageData>(id);
	auto graphicsData = registry->Get<GraphicsData>(id);
	data->thisID = id;
	graphicsData->renderingData.renderType = RENDER_TYPE::QUAD;
	graphicsData->renderingData.passType = PASS_TYPE::UI;
	graphicsData->renderingData.sprite.TilingX = 1.0f;
	graphicsData->renderingData.sprite.TilingY = 1.0f;
	graphicsData->renderingData.sprite.OffsetX = 0.0f;
	graphicsData->renderingData.sprite.OffsetY = 0.0f;
	graphicsData->renderingData.sprite.Order = 0;
	graphicsData->renderingData.sprite.color[0] = 1.0f;
	graphicsData->renderingData.sprite.color[1] = 1.0f;
	graphicsData->renderingData.sprite.color[2] = 1.0f;
	graphicsData->renderingData.sprite.color[3] = 1.0f;
	return data;
}

UIImageData* UIImage_Get(unsigned int id)
{
	auto registry = GetRegistry();
	UIImageData* data = registry->Get<UIImageData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[UIImage][GetComponent] Not Find Component");
	}
	return data;
}

bool UIImage_Has(unsigned int id)
{
	auto registry = GetRegistry();
	UIImageData* data = registry->Get<UIImageData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void UIImage_SetTexture(unsigned int id, const char* name)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UIImageData>(id);
	auto data2 = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		std::string textureName(name);
		auto textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
		data2->renderingData.texture_key = textureID;
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetTexture] Not Find Component");
	}
}

void UIImage_SetOrder(unsigned int id, int order)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UIImageData>(id);
	auto data2 = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		data2->renderingData.sprite.Order = order;
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetOrder] Not Find Component");
	}
}

void UIImage_SetColor(unsigned int id, float r, float g, float b, float a)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UIImageData>(id);
	auto data2 = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		auto normalizeColor = [](float value)
			{
				return (std::clamp)(value, 0.0f, 255.0f) / 255.0f;
			};

		data2->renderingData.sprite.color[0] = normalizeColor(r);
		data2->renderingData.sprite.color[1] = normalizeColor(g);
		data2->renderingData.sprite.color[2] = normalizeColor(b);
		data2->renderingData.sprite.color[3] = normalizeColor(a);
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetColor] Not Find Component");
	}
}
