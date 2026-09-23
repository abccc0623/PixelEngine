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
	graphicsData->renderingData.sprite.isShared = false;
	graphicsData->renderingData.sprite.TilingX = 1.0f;
	graphicsData->renderingData.sprite.TilingY = 1.0f;
	graphicsData->renderingData.sprite.OffsetX = 0.0f;
	graphicsData->renderingData.sprite.OffsetY = 0.0f;
	graphicsData->renderingData.sprite.Order = 0;
	graphicsData->renderingData.sprite.color[0] = 1.0f;
	graphicsData->renderingData.sprite.color[1] = 1.0f;
	graphicsData->renderingData.sprite.color[2] = 1.0f;
	graphicsData->renderingData.sprite.color[3] = 1.0f;
	graphicsData->renderingData.sprite.width = 0;
	graphicsData->renderingData.sprite.height = 0;
	graphicsData->renderingData.sprite.pivotX = 0.5f;
	graphicsData->renderingData.sprite.pivotY = 0.5f;
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

void UIImage_SetTextureSize(unsigned int id, float width, float height)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UIImageData>(id);
	auto data2 = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		data2->renderingData.sprite.width = width;
		data2->renderingData.sprite.height = height;
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetTextureSize] Not Find Component");
	}
}

bool UIImage_SetAspectFit(unsigned int id, float slotWidth, float slotHeight)
{
	auto registry = GetRegistry();
	auto image = registry->Get<UIImageData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (image == nullptr || graphics == nullptr)
	{
		PixelLog::Error("[UIImage][SetAspectFit] Not Find Component");
		return false;
	}

	if (slotWidth <= 0.0f || slotHeight <= 0.0f)
	{
		PixelLog::Error("[UIImage][SetAspectFit] Slot size must be greater than zero");
		return false;
	}

	uint32_t textureWidth = 0;
	uint32_t textureHeight = 0;
	if (!GetGraphicsTextureSize(static_cast<uint16_t>(graphics->renderingData.texture_key), &textureWidth, &textureHeight))
	{
		PixelLog::Error("[UIImage][SetAspectFit] Cannot get texture size");
		return false;
	}

	const float fitScale = (std::min)(slotWidth / static_cast<float>(textureWidth),
		slotHeight / static_cast<float>(textureHeight));
	graphics->renderingData.sprite.width = static_cast<float>(textureWidth) * fitScale;
	graphics->renderingData.sprite.height = static_cast<float>(textureHeight) * fitScale;
	return true;
}

void UIImage_SetPivot(unsigned int id, float x, float y)
{
	auto registry = GetRegistry();
	auto image = registry->Get<UIImageData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (image == nullptr || graphics == nullptr)
	{
		PixelLog::Error("[UIImage][SetPivot] Not Find Component");
		return;
	}

	graphics->renderingData.sprite.pivotX = (std::clamp)(x, 0.0f, 1.0f);
	graphics->renderingData.sprite.pivotY = (std::clamp)(y, 0.0f, 1.0f);
}

void UIImage_SetTiling(unsigned int id, float x, float y)
{
	auto registry = GetRegistry();
	auto image = registry->Get<UIImageData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (image == nullptr || graphics == nullptr)
	{
		PixelLog::Error("[UIImage][SetTiling] Not Find Component");
		return;
	}

	graphics->renderingData.sprite.TilingX = x;
	graphics->renderingData.sprite.TilingY = y;
}

void UIImage_SetOffset(unsigned int id, float x, float y)
{
	auto registry = GetRegistry();
	auto image = registry->Get<UIImageData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (image == nullptr || graphics == nullptr)
	{
		PixelLog::Error("[UIImage][SetOffset] Not Find Component");
		return;
	}

	graphics->renderingData.sprite.OffsetX = x;
	graphics->renderingData.sprite.OffsetY = y;
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
