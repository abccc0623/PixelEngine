#include "pch.h"
#include "Graphics.h"
#include "Renderer2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
extern PixelEngine* Engine;
Renderer2DData* Renderer2D_Add(unsigned int id)
{
	auto registry = GetRegistry();
	if (!registry->TryAddComponents<Renderer2DData, GraphicsData>(id)) return nullptr;
	auto graphics = registry->Get<GraphicsData>(id);
	graphics->renderingData.renderType = RENDER_TYPE::QUAD;
	graphics->renderingData.passType = PASS_TYPE::SCENE;
	graphics->renderingData.transparent = false;
	graphics->renderingData.sprite.Order = 0;
	graphics->renderingData.sprite.TilingX = 1.0f;
	graphics->renderingData.sprite.TilingY = 1.0f;
	graphics->renderingData.sprite.OffsetX = 0.0f;
	graphics->renderingData.sprite.OffsetY = 0.0f;
	graphics->renderingData.sprite.pivotX = 0.5f;
	graphics->renderingData.sprite.pivotY = 0.5f;
	graphics->renderingData.sprite.color[0] = 1.0f;
	graphics->renderingData.sprite.color[1] = 1.0f;
	graphics->renderingData.sprite.color[2] = 1.0f;
	graphics->renderingData.sprite.color[3] = 1.0f;

	auto renderer2D = registry->Get<Renderer2DData>(id);
	return renderer2D;
}
Renderer2DData* Renderer2D_Get(unsigned int id)
{
	auto registry = GetRegistry();
	Renderer2DData* data = registry->Get<Renderer2DData>(id);
	if (data == nullptr)
	{
		PixelLog::EntityError(id, "[Renderer2D][GetComponent] Not Find Component");
	}
	return data;
}
bool Renderer2D_Has(unsigned int id)
{
	auto registry = GetRegistry();
	Renderer2DData* data = registry->Get<Renderer2DData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Renderer2D_SetTexture(unsigned int id, const char* name)
{
	auto registry = GetRegistry();
	auto data1 = registry->Get<Renderer2DData>(id);
	auto data2 = registry->Get<GraphicsData>(id);
	if (data1 != nullptr)
	{
		std::string textureName(name);
		auto textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
		data2->renderingData.texture_key = textureID;
	}
	else
	{
		PixelLog::EntityError(id, "[Renderer2D][SetTexture] Not Find Component");
	}
}

void Renderer2D_SetColor(unsigned int id, float r, float g, float b, float a)
{
	auto registry = GetRegistry();
	auto renderer = registry->Get<Renderer2DData>(id);
	auto graphics = registry->Get<GraphicsData>(id);
	if (renderer == nullptr || graphics == nullptr)
	{
		PixelLog::EntityError(id, "[Renderer2D][SetColor] Not Find Component");
		return;
	}

	auto normalizeColor = [](float value)
		{
			return (std::clamp)(value, 0.0f, 255.0f) / 255.0f;
		};

	graphics->renderingData.sprite.color[0] = normalizeColor(r);
	graphics->renderingData.sprite.color[1] = normalizeColor(g);
	graphics->renderingData.sprite.color[2] = normalizeColor(b);
	graphics->renderingData.sprite.color[3] = normalizeColor(a);
}

void Renderer2D_SetTransparent(unsigned int id, bool transparent)
{
	auto registry = GetRegistry();
	auto graphics = registry->Get<GraphicsData>(id);
	if (graphics == nullptr || !registry->Has<Renderer2DData>(id))
	{
		PixelLog::EntityError(id, "[Renderer2D][SetTransparent] Not Find Component");
		return;
	}
	graphics->renderingData.transparent = transparent;
}
