#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "PixelResources.h"
#include "ResourceFactory.h"

namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceManager;

	class MaterialFactory : public ResourceFactory
	{
	public:
		explicit MaterialFactory(ResourceManager* resourceManager);

		bool Initialize(GraphicsCore* graphicsCore) override;
		void Release() override;
		void Clear() override;
		std::uint16_t Load(const std::string& path) override;
		MaterialResources* Get(std::uint16_t key);

	private:
		std::uint16_t AllocateKey();

		GraphicsCore* graphicsCore = nullptr;
		ResourceManager* resourceManager = nullptr;
		std::uint16_t defaultMaterialKey = 0;
		std::uint16_t nextMaterialKey = 1;
		std::unordered_map<std::uint16_t, MaterialResources> materials;
	};
}
