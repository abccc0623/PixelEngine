#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <d3d11.h>
#include <wrl/client.h>
#include "PixelResources.h"
#include "ResourceFactory.h"

using namespace Microsoft::WRL;
namespace PixelGraphics
{
	class GraphicsCore;
	class TextureFactory : public ResourceFactory
	{
	public:
		bool Initialize(GraphicsCore* graphicsCore) override;
		void Release() override;
		void Clear() override;
		std::uint16_t Load(const std::string& path) override;

		TextureResources* Get(std::uint16_t key);
	private:
		bool LoadDefaultTexture();
		std::uint16_t AllocateKey();

		GraphicsCore* graphicsCore = nullptr;
		std::uint16_t defaultTextureKey = 0;
		std::uint16_t nextTextureKey = 1;
		std::unordered_map<std::uint16_t, TextureResources> textures;
	};
}

