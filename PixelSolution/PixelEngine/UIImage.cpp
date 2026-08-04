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
	graphicsData->renderingData.sprite.Color[0] = 1.0f;
	graphicsData->renderingData.sprite.Color[1] = 1.0f;
	graphicsData->renderingData.sprite.Color[2] = 1.0f;
	graphicsData->renderingData.sprite.Color[3] = 1.0f;
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
