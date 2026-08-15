#include "pch.h"
#include "UISilder.h"
#include "PixelEngineAPI.h"
#include "PixelEngine.h"
#include "Registry.h"
#include "ResourceManager.h"
#include "Transform.h"

extern PixelEngine* Engine;

namespace
{
	void SetColor(float* color, float r, float g, float b, float a)
	{
		auto normalize = [](float component)
			{
				return (std::clamp)(component, 0.0f, 255.0f) / 255.0f;
			};

		color[0] = normalize(r);
		color[1] = normalize(g);
		color[2] = normalize(b);
		color[3] = normalize(a);
	}

	void InitializeQuad(RenderingData& renderingData, int order)
	{
		renderingData.renderType = RENDER_TYPE::QUAD;
		renderingData.passType = PASS_TYPE::UI;
		renderingData.sprite.isShared = false;
		renderingData.sprite.TilingX = 1.0f;
		renderingData.sprite.TilingY = 1.0f;
		renderingData.sprite.OffsetX = 0.0f;
		renderingData.sprite.OffsetY = 0.0f;
		renderingData.sprite.Order = order;
		renderingData.sprite.width = 100.0f;
		renderingData.sprite.height = 16.0f;
		renderingData.sprite.pivotX = 0.0f;
		renderingData.sprite.pivotY = 0.5f;
		SetColor(renderingData.sprite.color, 255.0f, 255.0f, 255.0f, 255.0f);
	}

	void UpdateFill(UISilderData& data)
	{
		const float range = data.maxValue - data.minValue;
		const float normalized = range > 0.0f
			? (data.value - data.minValue) / range
			: 0.0f;
		data.fill.sprite.width = data.width * (std::clamp)(normalized, 0.0f, 1.0f);
		data.fill.sprite.height = data.height;
		data.background.sprite.width = data.width;
		data.background.sprite.height = data.height;
	}

	UISilderData* FindUISilder(unsigned int id)
	{
		return GetRegistry()->Get<UISilderData>(id);
	}
}

UISilderData* UISilder_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<UISilderData>(id);
	if (!registry->Has<TransformData>(id))
	{
		Transform_Add(id);
	}

	auto data = registry->Get<UISilderData>(id);
	data->thisID = id;
	data->minValue = 0.0f;
	data->maxValue = 1.0f;
	data->value = 1.0f;
	data->width = 100.0f;
	data->height = 16.0f;
	InitializeQuad(data->background, 0);
	InitializeQuad(data->fill, 1);
	SetColor(data->background.sprite.color, 48.0f, 56.0f, 68.0f, 255.0f);
	SetColor(data->fill.sprite.color, 255.0f, 214.0f, 0.0f, 255.0f);
	UpdateFill(*data);
	return data;
}

UISilderData* UISilder_Get(unsigned int id)
{
	auto data = FindUISilder(id);
	if (data == nullptr)
	{
		PixelLog::Error("[UISilder][GetComponent] Not Find Component");
	}
	return data;
}

bool UISilder_Has(unsigned int id)
{
	return FindUISilder(id) != nullptr;
}

void UISilder_SetRange(unsigned int id, float minValue, float maxValue)
{
	auto data = FindUISilder(id);
	if (data == nullptr || maxValue <= minValue)
	{
		PixelLog::Error("[UISilder][SetRange] Invalid Component Or Range");
		return;
	}
	data->minValue = minValue;
	data->maxValue = maxValue;
	data->value = (std::clamp)(data->value, minValue, maxValue);
	UpdateFill(*data);
}

void UISilder_SetValue(unsigned int id, float value)
{
	auto data = FindUISilder(id);
	if (data == nullptr)
	{
		PixelLog::Error("[UISilder][SetValue] Not Find Component");
		return;
	}
	data->value = (std::clamp)(value, data->minValue, data->maxValue);
	UpdateFill(*data);
}

void UISilder_SetSize(unsigned int id, float width, float height)
{
	auto data = FindUISilder(id);
	if (data == nullptr || width < 0.0f || height < 0.0f)
	{
		PixelLog::Error("[UISilder][SetSize] Invalid Component Or Size");
		return;
	}
	data->width = width;
	data->height = height;
	UpdateFill(*data);
}

void UISilder_SetBackgroundColor(unsigned int id, float r, float g, float b, float a)
{
	auto data = FindUISilder(id);
	if (data != nullptr) SetColor(data->background.sprite.color, r, g, b, a);
}

void UISilder_SetFillColor(unsigned int id, float r, float g, float b, float a)
{
	auto data = FindUISilder(id);
	if (data != nullptr) SetColor(data->fill.sprite.color, r, g, b, a);
}

void UISilder_SetBackgroundTexture(unsigned int id, const char* name)
{
	auto data = FindUISilder(id);
	if (data != nullptr) data->background.texture_key = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, name);
}

void UISilder_SetFillTexture(unsigned int id, const char* name)
{
	auto data = FindUISilder(id);
	if (data != nullptr) data->fill.texture_key = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, name);
}

void UISilder_SetOrder(unsigned int id, int order)
{
	auto data = FindUISilder(id);
	if (data == nullptr) return;
	data->background.sprite.Order = order;
	data->fill.sprite.Order = order + 1;
}
