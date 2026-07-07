#include "pch.h"
#include "Renderer2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
extern PixelEngine* Engine;
Renderer2DData* Renderer2D_Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<Renderer2DData>(id);

	if (registry->Has<GraphicsData>(id) == false)
	{
		registry->Add<GraphicsData>(id);
	}


	auto data1 = registry->Get<Renderer2DData>(id);
	auto data2 = registry->Get<GraphicsData>(id);
	data2->renderingData.Type = QUAD;
	return data1;
}
Renderer2DData* Renderer2D_Get(unsigned int id)
{
	auto registry = GetRegistry();
	Renderer2DData* data = registry->Get<Renderer2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Transform][GetComponent] Not Find Component");
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
		PixelLog::Error("[Renderer2D][SetTexture] Not Find Component");
	}
}

