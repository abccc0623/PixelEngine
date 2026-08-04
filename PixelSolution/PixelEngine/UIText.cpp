#include "pch.h"
#include "UIText.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "Graphics.h"
#include <algorithm>
#include <cstring>
#include "PixelEngine.h"
#include "ResourceManager.h"
extern PixelEngine* Engine;
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
	graphicsData->renderingData.text.color[0] = 1.0f;
	graphicsData->renderingData.text.color[1] = 1.0f;
	graphicsData->renderingData.text.color[2] = 1.0f;
	graphicsData->renderingData.text.color[3] = 1.0f;
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

void UIText_SetText(unsigned int id, const char* text)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UITextData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (data1 != nullptr && graphics != nullptr)
	{
		auto& target = graphics->renderingData.text;
		std::memset(target.content, 0, sizeof(target.content));

		const std::string textSTR = text != nullptr ? text : "";
		const size_t length = (std::min)(textSTR.size(), sizeof(target.content) - 1);
		if (length > 0)
		{
			std::memcpy(target.content, textSTR.data(), length);
		}

		data1->text = textSTR;
	}
	else
	{
		PixelLog::Error("[UIText][SetText] Not Find Component");
	}
}

void UIText_SetTextSize(unsigned int id, float scale)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UITextData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		graphics->renderingData.text.scale = scale;
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetTextSize] Not Find Component");
	}
}

void UIText_SetFont(unsigned int id, const char* name)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<UITextData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		std::string fontName(name);
		auto textureID = Engine->GetResourceID(RESOURCE_TYPE::FONT, fontName);
		graphics->renderingData.text.fontKey = textureID;
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetFont] Not Find Component");
	}
}

void UIText_SetColor(unsigned int id, float r, float g, float b, float a)
{
	auto registry = GetRegistry();
	auto textData = registry->Get<UITextData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (textData == nullptr || graphics == nullptr)
	{
		PixelLog::Error("[UIText][SetColor] Not Find Component");
		return;
	}

	auto normalizeColor = [](float value)
		{
			return (std::clamp)(value, 0.0f, 255.0f) / 255.0f;
		};

	graphics->renderingData.text.color[0] = normalizeColor(r);
	graphics->renderingData.text.color[1] = normalizeColor(g);
	graphics->renderingData.text.color[2] = normalizeColor(b);
	graphics->renderingData.text.color[3] = normalizeColor(a);
}
