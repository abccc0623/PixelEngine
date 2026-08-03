#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "PixelResources.h"
#include "ResourceFactory.h"

namespace PixelGraphics
{
	class GraphicsCore;

	class FontFactory : public ResourceFactory
	{
	public:
		bool Initialize(GraphicsCore* graphicsCore) override;
		void Release() override;
		void Clear() override;
		std::uint16_t Load(const std::string& path) override;
		FontResources* Get(std::uint16_t key);

	private:
		bool LoadDefaultFont();
		bool CreateFontResource(const std::vector<unsigned char>& fontFileData, std::uint16_t key, const std::string& path);
		std::uint16_t AllocateKey();

		GraphicsCore* graphicsCore = nullptr;
		std::uint16_t defaultFontKey = 0;
		std::uint16_t nextFontKey = 1;
		std::vector<unsigned char> defaultFontFileData;
		std::unordered_map<std::uint16_t, FontResources> fonts;
	};
}
