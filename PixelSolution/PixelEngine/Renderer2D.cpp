#include "pch.h"
#include "Renderer2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "ResourceManager.h"
extern PixelEngine* Engine;
void* ECS::Renderer2D::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Renderer2DData>(id);
	auto data = registry->Get<Renderer2DData>(id);
	data->renderingData->Type = QUAD;
	return data;
}
void* ECS::Renderer2D::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	Renderer2DData* data = registry->Get<Renderer2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Transform][GetComponent] Not Find Component");
	}
	return data;
}
bool ECS::Renderer2D::HasComponent(unsigned int id)
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
std::string ECS::Renderer2D::BindJit()
{
	std::string jit = R"(
	---@class Renderer2DData
	---@field TilingX number
	---@field TilingY number
	---@field OffsetX number
	---@field OffsetY number
ffi.cdef[[
    typedef struct 
    { 
        const void* const renderingData;
        float TilingX; 
        float TilingY; 
        float OffsetX; 
        float OffsetY; 
    } Renderer2DData;
]]
    )";
	return jit;
}

void ECS::Renderer2D::SetTexture(unsigned int id, const char* name)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Renderer2DData>(id);
	if (data != nullptr)
	{
		std::string textureName(name);
		auto textureID = Engine->GetResourceID(RESOURCE_TYPE::TEXTURE, textureName);
		data->renderingData->texture_key = textureID;
	}
	else
	{
		PixelLog::Error("[Renderer2D][SetTexture] Not Find Component");
	}
}

