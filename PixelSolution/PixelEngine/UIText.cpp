#include "pch.h"
#include "UIText.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "Graphics.h"

UITextData* UIText_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<UITextData>(id);
	if (registry->Has<GraphicsData>(id) == false)
	{
		registry->Add<GraphicsData>(id);
	}
	auto data = registry->Get<UITextData>(id);
	auto graphicsData = registry->Get<GraphicsData>(id);
	data->thisID = id;
	graphicsData->renderingData.renderType = RENDER_TYPE::TEXT;
	graphicsData->renderingData.passType = PASS_TYPE::UI;
	graphicsData->renderingData.sprite.TilingX = 1.0f;
	graphicsData->renderingData.sprite.TilingY = 1.0f;
	graphicsData->renderingData.sprite.OffsetX = 0.0f;
	graphicsData->renderingData.sprite.OffsetY = 0.0f;
	graphicsData->renderingData.sprite.Order = 0;
	return data;
}

UITextData* UIText_Get(unsigned int id)
{
	auto registry = GetRegistry();
	UITextData* data = registry->Get<UITextData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[UIText][GetComponent] Not Find Component");
	}
	return data;
}

bool UIText_Has(unsigned int id)
{
	auto registry = GetRegistry();
	return registry->Get<UITextData>(id) != nullptr;
}
