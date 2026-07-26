#include "pch.h"
#include "MaterialFactory.h"

#include <fstream>
#include <sstream>

#include "GraphicsCore.h"
#include "ResourceManager.h"

PixelGraphics::MaterialFactory::MaterialFactory(ResourceManager* manager) : resourceManager(manager)
{
}

bool PixelGraphics::MaterialFactory::Initialize(GraphicsCore* core)
{
	Clear();
	graphicsCore = core;
	defaultMaterialKey = static_cast<std::uint16_t>(ResourceDefaultKey::MATERIAL_DEFAULT);

	MaterialResources material = {};
	material.key = defaultMaterialKey;
	material.path = "DEFAULT_MATERIAL";
	material.TextureKey = static_cast<std::uint16_t>(ResourceDefaultKey::TEXTURE);
	material.ShaderKey = static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_STATIC);
	material.RasterizerStateKey = static_cast<std::uint16_t>(ResourceDefaultKey::RASTERIZER_SOLID);
	materials.emplace(defaultMaterialKey, std::move(material));
	return true;
}

void PixelGraphics::MaterialFactory::Release()
{
	Clear();
	graphicsCore = nullptr;
}

void PixelGraphics::MaterialFactory::Clear()
{
	materials.clear();
	nextMaterialKey = 1;
}

std::uint16_t PixelGraphics::MaterialFactory::AllocateKey()
{
	constexpr std::uint16_t invalidKey = (std::numeric_limits<std::uint16_t>::max)();

	for (std::uint32_t count = 0; count < invalidKey - 1; ++count)
	{
		if (nextMaterialKey == defaultMaterialKey || nextMaterialKey == invalidKey)
		{
			nextMaterialKey = 1;
		}

		const std::uint16_t key = nextMaterialKey++;
		if (materials.find(key) == materials.end())
		{
			return key;
		}
	}

	return invalidKey;
}

std::uint16_t PixelGraphics::MaterialFactory::Load(const std::string& path)
{
	for (const auto& [key, material] : materials)
	{
		if (material.path == path)
		{
			return key;
		}
	}

	std::ifstream file(path);
	if (!file.is_open())
	{
		return defaultMaterialKey;
	}

	MaterialResources material = {};
	material.path = path;
	material.TextureKey = static_cast<std::uint16_t>(ResourceDefaultKey::TEXTURE);
	material.ShaderKey = static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_STATIC);
	material.RasterizerStateKey = static_cast<std::uint16_t>(ResourceDefaultKey::RASTERIZER_SOLID);

	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string property;
		std::string value;
		stream >> property >> value;
		if (property.empty() || value.empty())
		{
			continue;
		}

		if ((property == "@Texture" || property == "@Path") && resourceManager)
		{
			material.TextureKey = resourceManager->Load(ResourceType::TEXTURE, value.c_str());
		}
		else if (property == "@Shader" && resourceManager)
		{
			material.ShaderKey = resourceManager->Load(ResourceType::SHADER, value.c_str());
		}
		else if (property == "@Rasterizer" && resourceManager)
		{
			material.RasterizerStateKey = resourceManager->Load(ResourceType::RASTERIZER_STATE, value.c_str());
		}
		else if (property == "@OffsetX")
		{
			material.Offset[0] = std::stof(value);
		}
		else if (property == "@OffsetY")
		{
			material.Offset[1] = std::stof(value);
		}
		else if (property == "@TilingX")
		{
			material.Tiling[0] = std::stof(value);
		}
		else if (property == "@TilingY")
		{
			material.Tiling[1] = std::stof(value);
		}
	}

	const std::uint16_t key = AllocateKey();
	constexpr std::uint16_t invalidKey = (std::numeric_limits<std::uint16_t>::max)();
	if (key == invalidKey)
	{
		return defaultMaterialKey;
	}

	material.key = key;
	materials.emplace(key, std::move(material));
	return key;
}

MaterialResources* PixelGraphics::MaterialFactory::Get(std::uint16_t key)
{
	const auto found = materials.find(key);
	if (found != materials.end())
	{
		return &found->second;
	}

	const auto defaultMaterial = materials.find(defaultMaterialKey);
	if (defaultMaterial != materials.end())
	{
		return &defaultMaterial->second;
	}

	return nullptr;
}
