#include "pch.h"
#include "ResourceManager.h"

bool PixelGraphics::ResourceManager::Initialize(GraphicsCore* graphicsCore)
{
	Release();
	factoryMap.emplace(ResourceType::TEXTURE, new TextureFactory());
	factoryMap.emplace(ResourceType::SHADER, new ShaderFactory());
	factoryMap.emplace(ResourceType::RASTERIZER_STATE, new RasterizerStateFactory());
	factoryMap.emplace(ResourceType::MODEL, new ModelFactory());
	factoryMap.emplace(ResourceType::MATERIAL, new MaterialFactory(this));
	factoryMap.emplace(ResourceType::FONT, new FontFactory());

	for (auto& K : factoryMap)
	{
		if (!K.second || !K.second->Initialize(graphicsCore))
		{
			Release();
			return false;
		}
	}
	return true;
}

void PixelGraphics::ResourceManager::Release()
{
	for (auto& K : factoryMap)
	{
		if (K.second)
		{
			K.second->Release();
			delete K.second;
			K.second = nullptr;
		}
	}
	factoryMap.clear();
}

void PixelGraphics::ResourceManager::Clear()
{
	for (auto& K : factoryMap)
	{
		if (K.second)
		{
			K.second->Clear();
		}
	}
}

uint16_t PixelGraphics::ResourceManager::Load(ResourceType type, const char* path)
{
	std::string ResourcePath(path);
	const auto factory = factoryMap.find(type);
	if (factory == factoryMap.end() || !factory->second)
	{
		return 0;
	}

	return factory->second->Load(ResourcePath);
}
